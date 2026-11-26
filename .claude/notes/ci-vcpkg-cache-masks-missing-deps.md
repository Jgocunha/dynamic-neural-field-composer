# The CI vcpkg cache used to hide missing dependencies - and why it no longer can

Until 3.0.0 every vcpkg-using job cloned vcpkg HEAD, installed packages in classic mode, and
restored vcpkg's whole `installed/` tree from the Actions cache with a prefix-only
`restore-keys` fallback. A package that had once been installed stayed in that restored tree
after it was dropped from the install list, so anything still depending on it kept working
until the cache was evicted (7 days unused) - and then every gap surfaced at once, in a
commit that did not cause it. That happened on 2026-09-24 (Catch2, needed by
`imgui-platform-kit`'s tests, and `autoconf-archive` for vcpkg's `pthread-stubs`; see the
2.12.1 CHANGELOG entry).

Since 3.0.0 the dependencies are a vcpkg manifest (`vcpkg.json`), and CI caches vcpkg's
**binary cache** (`VCPKG_DEFAULT_BINARY_CACHE`), not `installed/`:

- Every configure installs exactly the manifest's dependency set into a fresh
  `<build tree>/vcpkg_installed/`. Nothing outside the manifest can be present.
- Binary packages are zip files named by their ABI hash (port files, triplet, compiler,
  CMake version, ...). A restored older cache only supplies packages whose hash matches;
  anything else is rebuilt. Restoring cannot change which packages, or which versions,
  end up installed.
- It can still hide a missing **system** prerequisite. A cached package is not rebuilt, so
  the build tools its port needs are never run. `pthread-stubs` needs `autoconf`,
  `autoconf-archive`, `automake` and `libtool` from apt; with a warm cache a workflow
  that lacks them passes, and it fails as soon as the cache is cold or the package's hash
  changes. That is how imgui-platform-kit's Linux workflows failed on their first
  manifest-mode run.

Mechanics (`.github/actions/setup-vcpkg/action.yml`): restore with a never-matching
`run_id` key plus the `vcpkg-bin-<os>-<triplet>-` prefix (newest wins); after configure,
`actions/cache/save` with `<prefix><hashFiles of the cache dir>`. An unchanged cache hashes
to an existing key and the save is a no-op; new packages (a runner-image compiler or CMake
update changes the ABI hash) produce a new key. Old archives are never pruned - the cache
grows until GitHub evicts it, which only costs one full rebuild.

When a dependency failure shows up on an unrelated commit now, suspect the runner image
(apt packages vcpkg needs, compiler) first: the vcpkg side is pinned. Check the "Restore
vcpkg binary cache" step for a miss, and look in the vcpkg output for the port that was
built rather than restored; that port is the one whose prerequisites are missing. To
reproduce locally, install with an empty cache: `vcpkg install --binarysource=clear`.

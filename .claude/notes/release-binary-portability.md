# Release binary portability

What a prebuilt release archive (`.github/workflows/release.yml`) needs from the machine
it runs on is set by where and how CI builds it, not by the code. Each release job ends
with a check step that fails the job when its binaries need more than the minimum below.

| Archive | Minimum | What sets it |
|---|---|---|
| linux-x64 | glibc 2.35 (Ubuntu 22.04) | Built in an `ubuntu:22.04` container, gcc-13 from `ppa:ubuntu-toolchain-r/test`, `-static-libstdc++ -static-libgcc` |
| macos-arm64 | macOS 13.3 | `MACOSX_DEPLOYMENT_TARGET` in the job env (the project) and `.github/vcpkg-triplets/arm64-osx-portable.cmake` (the dependencies) - keep them equal |
| windows-x64 | Windows 10 | `fftw3.dll` and the MSVC runtime installed into `bin/` by the `if(WIN32)` block in `CMakeLists.txt` |

Why each one is the way it is:

- **glibc.** A binary needs at least the glibc it was linked against. Building against
  glibc 2.38+ also pulls in `__isoc23_strtol`/`__isoc23_sscanf` and the 2.38 `fmod`, so
  the code cannot avoid it. glibc cannot be linked statically into a GLFW/OpenGL app, so
  the oldest distro you build on is the oldest you support. The `ubuntu-22.04` runner
  image retires in 2027, hence the container.
- **libstdc++.** gcc-13's libstdc++ needs `GLIBCXX_3.4.32`, which a stock 22.04 lacks, so it
  is linked statically.
- **macOS 13.3, not 13.0.** Public headers use `std::format` with floating point; libc++
  marks that available from macOS 13.3.
- **vcpkg triplet, not just the env var.** vcpkg's ABI hash covers the triplet file, not the
  environment, so changing only `MACOSX_DEPLOYMENT_TARGET` would let the binary cache hand
  back dependencies built for the runner's macOS. An object built for a newer macOS only
  warns at link time, so the check scans the static libraries too, not just the executables.
- **Manifest mode ignores `VCPKG_DEFAULT_TRIPLET`.** The toolchain picks the triplet from
  `VCPKG_TARGET_TRIPLET` or the host, so the macOS job passes `-DVCPKG_TARGET_TRIPLET` and
  `-DVCPKG_OVERLAY_TRIPLETS` explicitly.
- **Container jobs:** `${{ github.workspace }}` is the host path; use `$GITHUB_WORKSPACE`.

Checking an archive by hand, without running it:

- Linux: `objdump -T <bin> | grep -o 'GLIBC_2\.[0-9]*' | sort -uV | tail -1`
- Windows: `objdump -p <exe> | grep 'DLL Name'` (or `dumpbin -dependents`)
- macOS: `otool -l <bin> | grep minos`

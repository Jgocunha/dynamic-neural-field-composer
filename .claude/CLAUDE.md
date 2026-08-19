# CLAUDE.md

## 1. Think Before Coding

**Don't assume. Don't hide confusion. Surface tradeoffs.**

Before implementing:
- State your assumptions explicitly. If uncertain, ask.
- If multiple interpretations exist, present them - don't pick silently.
- If a simpler approach exists, say so. Push back when warranted.
- If something is unclear, stop. Name what's confusing. Ask.

## 2. Simplicity First

**Minimum code that solves the problem. Nothing speculative.**

- No features beyond what was asked.
- No abstractions for single-use code.
- No "flexibility" or "configurability" that wasn't requested.
- No error handling for impossible scenarios.
- If you write 200 lines and it could be 50, rewrite it.

Ask yourself: "Would a senior engineer say this is overcomplicated?" If yes, simplify.

## 3. Surgical Changes

**Touch only what you must. Clean up only your own mess.**

When editing existing code:
- Don't "improve" adjacent code, comments, or formatting.
- Don't refactor things that aren't broken.
- Match existing style, even if you'd do it differently.
- If you notice unrelated dead code, mention it - don't delete it.

When your changes create orphans:
- Remove imports/variables/functions that YOUR changes made unused.
- Don't remove pre-existing dead code unless asked.

The test: Every changed line should trace directly to the user's request.

## 4. Goal-Driven Execution

**Define success criteria. Loop until verified.**

Transform tasks into verifiable goals:
- "Add validation" → "Write tests for invalid inputs, then make them pass"
- "Fix the bug" → "Write a test that reproduces it, then make it pass"
- "Refactor X" → "Ensure tests pass before and after"

For multi-step tasks, state a brief plan:
```
1. [Step] → verify: [check]
2. [Step] → verify: [check]
3. [Step] → verify: [check]
```

Strong success criteria let you loop independently. Weak criteria ("make it work") require constant clarification.

---

# Project: dynamic-neural-field-composer

Everything above is general engineering guidance. Everything below is specific to this
repository and takes precedence where the two overlap.

## Layout

The repo root holds `README.md`, `CHANGELOG.md`, `CONTRIBUTING.md`, `codecov.yml` and
`.github/`. The project itself is nested one level down:

    dynamic-neural-field-composer/
    |-- include/          public headers - Doxygen lives here
    |   |-- elements/ element_parameters/ simulation/ visualization/
    |   |-- user_interface/ application/
    |   |-- tools/        math.h utils.h profiling.h logger.h fft_convolution.h simd_dispatch.h
    |   +-- exceptions/   exception.h
    |-- src/              mirrors include/
    |-- tests/            GoogleTest -> target dnf_composer_tests
    |-- examples/         ~21 demo main()s, deliberately not unit-tested
    |-- wiki/             15 user-facing pages
    +-- scripts/          setup/build/install for windows, linux, macos

## Build and test

Use the `build-and-test` skill. Two facts that bite:

- **`tests/CMakeLists.txt` lists every test file explicitly. There is no globbing.** A new
  test file not added there is silently never compiled - the suite goes green without ever
  having run it.
- The live build tree is Ninja at `build/release`.

## Conventions

**Backwards compatibility is mandatory.** If a change cannot preserve it, stop and ask.
Never proceed on an assumption about what is safe to break.

**Clean Code always.** Deviating needs an explicit justification in the PR description.
Meaningful names, small focused functions, no comments explaining *what*, no raw owning
pointers (`shared_ptr` / `unique_ptr` only). See the code style section of
`CONTRIBUTING.md`.

**Check `tools/` before writing any helper.** Math, utility, profiling and exception
helpers have existing homes and belong there, not beside their call site:

| Need | Goes in |
|---|---|
| Math / numerics | `include/tools/math.h` |
| General utility | `include/tools/utils.h` |
| Profiling / timing | `include/tools/profiling.h` |
| Logging | `include/tools/logger.h` |
| Convolution | `include/tools/fft_convolution.h` |
| Exceptions, error codes | `include/exceptions/exception.h` |

Search these first. Re-implementing something that already exists in `tools/` is a review
failure, not a style nit.

**Modern C++20** - ranges, concepts, structured bindings, `std::span`, `std::string_view`,
`[[nodiscard]]`.

**TDD** - write the failing test first, watch it fail, then implement. "Fix the bug" means
"write a test that reproduces it, then make it pass".

**Instrument, don't guess.** When tracing a bug, add logging and read the output rather than
reasoning about control flow from the source. Use
`dnf_composer::tools::logger::log(logger::DEBUG, ...)` from `include/tools/logger.h` -
levels `DEBUG/INFO/WARNING/ERROR/FATAL`, `setMinLogLevel()` for the threshold, and
`tests/common/scoped_min_log_level.h` when a test needs to change it. Write messages as
plain natural-language sentences carrying the values that matter: *"field 'u' at step 42:
max activation 0.87 at index 51, below threshold 1.0, no bump"* is worth far more than
`here1` or a bare variable dump - people and models both read them. Remove instrumentation
before the PR unless it earns a permanent place at `DEBUG`.

**Write findings down.** Something learned while working that is not obvious from the code
goes in a short, self-contained `.md` file - one topic per file, readable without the
conversation that produced it:

| Finding | Goes in |
|---|---|
| True of the project anywhere | `.claude/(project)notes/` - tracked |
| Only true of this machine (toolchain versions, install locations, local paths) | `.claude/(machine)notes/` - gitignored |

**Temp files** go to `.claude/temp/` (gitignored; `mkdir -p` it if missing). Never scatter
scratch files through the project tree.

**Doxygen** - new or changed public entities under `include/` need `@brief`, one `@param`
per parameter, and `@return` unless void.

**Wiki** - user-visible changes (element parameters, UI behaviour, build process) require
the matching page under `wiki/` updated in the same PR.

## Naming

Branches - `bug/`, `feat/`, `chore/`, `ci/`, `docs/`, `test/`, `refactor/` plus a short
slug: `bug/connection-dim-check`, `feat/oja-example`.

Commits - conventional, lowercase, optional scope. Types in use across the history:
`fix, feat, test, perf, docs, refactor, ci, chore, style`.

    fix: reject 1D<->2D connections with matching flattened size
    test(golden): add sigmoid regime coverage
    chore: pin vcpkg and imgui-platform-kit revisions

Note the split: bugfix *branches* use `bug/`, their *commits* use `fix:`.

PRs - concise title, a short description of what changed / why / how to verify, and
`Closes #N`.

## Release (maintainers)

Version lives in `dynamic-neural-field-composer/CMakeLists.txt`
(`DNF_COMPOSER_VERSION_MAJOR` / `MINOR` / `PATCH`). Bump per SemVer, add a `CHANGELOG.md`
entry, commit `release: vX.Y.Z`, then tag and push - release CI triggers on the tag.

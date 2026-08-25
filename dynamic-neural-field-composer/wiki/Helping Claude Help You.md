# Helping Claude Help You

This repository ships a configured [Claude Code](https://claude.com/claude-code) setup in
its `.claude/` directory. Clone the project, open Claude, and it already knows the build
commands, the coding conventions, and the workflows this project expects — you do not have
to explain them.

This page covers what you inherit, what the project expects of AI-assisted changes, and the
one rule that has no exceptions: **Claude never merges.**

None of this is required. Contributions written entirely by hand are equally welcome, and
are held to exactly the same standard.

---

## What ships in `.claude/`

| Path | In the repo? | What it is |
|---|---|---|
| `CLAUDE.md` | yes | Project conventions. Claude reads this every session. |
| `skills/` | yes | Eight task workflows (below). |
| `notes/` | yes | Findings about the project that aren't obvious from the code. |
| `local-notes/` | no | Notes true of one machine only — toolchain versions, local paths. |
| `temp/` | no | Scratch space. Never commit anything from here. |
| `settings*.json` | no | Personal permission settings. |

The tracked half is shared deliberately, so everyone works from the same conventions. The
untracked half is yours and stays on your machine.

If you learn something while working that the code doesn't already say — a non-obvious
constraint, a subtlety in the build — write it into `notes/` as a short standalone
`.md`. That's what the directory is for.

---

## The skills

A skill is a packaged workflow. Describe your task and the matching one activates on its
own; you can also just name it.

| Skill | What it does |
|---|---|
| `build-and-test` | Configure, build and run the GoogleTest suite via the CMake presets. |
| `work-an-issue` | End-to-end for one issue: branch, failing test, implement, build, review, docs, PR. |
| `docs-check` | Checks Doxygen on public API, the matching wiki page, and stale references. |
| `project-code-review` | Reviews a diff against this project's standards before you push it. |
| `perf-regression-test` | Checks a hot-path diff against this machine's recorded benchmark baseline. |
| `commit` | Reads your uncommitted diff and writes the conventional commit message(s). |
| `pr` | Reads the branch diff and writes the PR title and description. |
| `autopilot` | Maintainer orchestration for unattended issue work. You will not need this. |

For the authoritative version of any of them, read `.claude/skills/<name>/SKILL.md`. This
page summarises; those files are the source of truth.

---

## What Claude will do here

These come from `.claude/CLAUDE.md`, and they are the same things a reviewer will look for
whether or not you used an AI:

- **Backwards compatibility is mandatory.** If a change can't preserve it, Claude is told to
  stop and ask rather than decide for you.
- **Check `tools/` before writing a helper.** Math, utility, profiling, logging and
  exception helpers have existing homes. Re-implementing one is a review failure, not a
  style nit.
- **Test-driven.** Write the failing test, watch it fail, then implement.
- **New test files must be added to `tests/CMakeLists.txt`.** There is no globbing — an
  unlisted file is silently never compiled, and the suite goes green having never run it.
- **Instrument, don't guess.** When tracing a bug, add logging via `tools::logger` and read
  the output instead of reasoning about control flow from the source.
- **Doxygen** on new or changed public entities under `include/`.
- **Wiki** page updated when behaviour changes visibly.
- Clean Code, modern C++20, no raw owning pointers.

---

## Getting good results

- **Give it a success criterion, not a goal.** "Make the connection check reject 1D↔2D
  pairs, with a test that fails before the fix" beats "fix the connection bug". Weak
  criteria produce work you have to redo.
- **Point at the issue number.** `work-an-issue 41` pulls the issue and its comments rather
  than working from your paraphrase of it.
- **Let it write the failing test first.** A test written after the fix tends to encode the
  fix rather than the requirement.
- **Ask for real output.** "Show me the test run" — not "did it pass?". Claude is instructed
  to report actual counts and paste real failures, so hold it to that.
- **One change per PR.** Same rule as any other contribution.
- **Push back.** If a change looks larger than the problem, say so. Claude is told to prefer
  the smallest change that works, and will drop the extra scope when challenged.

---

## Never let Claude merge

**Claude may branch, commit, push, and open a pull request. Merging is a human decision,
every time.**

Before anything lands:

- CI is green on Windows, Linux and macOS
- CodeRabbit's review comments are resolved
- **a human has read the diff**

That last one is not a formality. AI-assisted changes can be confidently wrong in ways that
compile and pass tests — a plausible-looking helper that duplicates something already in
`tools/`, a test that asserts the behaviour it just observed rather than the behaviour that
was specified, an edge case silently dropped.

You are the author of what you submit. "Claude wrote it" is not a review, and it is not a
defence in review.

---

## Before you open the PR

The [PR template](https://github.com/Jgocunha/dynamic-neural-field-composer/blob/main/.github/pull_request_template.md)
checklist applies unchanged:

- [ ] Tests added or updated
- [ ] Doxygen updated (if public API changed)
- [ ] Wiki updated (if user-visible behaviour changed)
- [ ] CI passes (Windows, Linux, macOS)

`project-code-review` and `docs-check` cover most of this before you push. Running them is
faster than finding out from CodeRabbit.

Mention in the PR description that the change was AI-assisted. It is not a mark against it —
it just tells the reviewer where to look harder.

---

## For maintainers

`autopilot` orchestrates unattended work across several issues at once: it triages open
issues, dispatches worker agents into separate worktrees, reviews the diffs, opens PRs and
watches CI until green. It stops at green and never merges. See
`.claude/skills/autopilot/SKILL.md`.

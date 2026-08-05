# Changelog

> **The repository-root [`CHANGELOG.md`](../CHANGELOG.md) is the single source of
> truth.** It carries the complete released history (currently through 2.9.6) and is
> the file every merged PR updates. This package-level file is kept only for the
> entry below and is not maintained per-release — add new entries to the root file,
> not here.

All notable changes to this project are documented here. The format is based on
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project
adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Golden algebraic-equivalence and characterization test suite covering every
  DNF element (1D and 2D) plus five composed architectures. Analytic elements
  are checked against independent reference implementations at `1e-9`; noise is
  validated as a statistical golden (k-sigma tolerance floor); trajectories are
  frozen as CSV fixtures. 39 golden suites, 177 frozen CSVs, full suite
  972/972 green ([#92], closes [#54]).

[Unreleased]: https://github.com/Jgocunha/dynamic-neural-field-composer/compare/v2.9.4...HEAD
[#92]: https://github.com/Jgocunha/dynamic-neural-field-composer/pull/92
[#54]: https://github.com/Jgocunha/dynamic-neural-field-composer/issues/54

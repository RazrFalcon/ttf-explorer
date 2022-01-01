# Change Log
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/)
and this project adheres to [Semantic Versioning](http://semver.org/).

## [Unreleased]
### Added
- `ankr`, `feat` and `trak` tables.

## [0.2.0] - 2021-12-31
### Added
- The **Size** column for groups.
- A dedicated array group type.
- Parser reads the file sequentially now, instead of jumping to offsets.
- All unsupported data will be explicitly added to the tree. Including paddings.
  Previously, it was ignored and the tree didn't had a precise structure.
- Name ID resolving. References to the `name` table will be resolved in-place.

### Changed
- Almost a complete rewrite.
- Parser rewritten back to C++ from Rust. Mainly to simplify deployment and
  optimize strings allocations.
- Many performance optimizations.

[Unreleased]: https://github.com/RazrFalcon/ttf-explorer/compare/v0.2.0...HEAD
[0.2.0]: https://github.com/RazrFalcon/rustybuzz/compare/v0.1.0...v0.2.0

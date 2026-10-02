# Contributing

Before opening new issues and contributing any code to the project, read our
[Code of Conduct](CODE_OF_CONDUCT.md) first.

## Finding something to work on
- Issues labelled [`good first issue`](https://github.com/veritaware/Besprited/issues?q=is%3Aissue+is%3Aopen+label%3A%22good+first+issue%22)
  are small and well scoped, a good place to start.
  [`help wanted`](https://github.com/veritaware/Besprited/issues?q=is%3Aissue+is%3Aopen+label%3A%22help+wanted%22)
  issues are a bit bigger but also open for contributors. Many of them have a "Where to start" comment
  pointing at the relevant code.
- **Comment on the issue to claim it before you start**, and wait until it's assigned to you. This avoids
  two people working on the same thing. Please claim one issue at a time. If there's no pull request or
  update within about 7 days, the issue may be handed to someone else.
- Want to work on something that has no issue yet? Open one first (bug report or feature request), so
  the approach can be agreed on before you write code.
- Questions are welcome on our [Discord](https://discord.gg/hTp5gPUUJT) or in the issue itself.

## Building and testing
- Follow [BUILDING.md](BUILDING.md) to get the sources (the repository uses submodules, so clone with
  `--recursive`) and the dependencies for your platform.
- To build and run the unit tests, configure with tests enabled and use CTest from the build directory:
  ```
  cmake -G Ninja -DENABLE_TESTS=on ..
  ninja
  ctest
  ```
  Add or update tests for the behaviour you change where possible. Unit tests live in `test/`, mirroring
  the `src/` layout, and every `*_tests.cpp` file becomes its own test executable.
- Code is formatted with clang-format using the repository's [`.clang-format`](.clang-format). Please
  format only the lines you changed (e.g. with `git clang-format`) rather than whole files, so diffs stay
  reviewable.

## Copyright headers
A CI check (`Copyright Header Check`) verifies every changed `.h`, `.hpp`, `.c`, `.cpp`, `.xml` and
`CMakeLists.txt` file. Within its first 10 lines it must contain a Veritaware copyright line covering the
**current year**:
- When modifying an existing file, add or update a Besprited line aligned with the existing author lines,
  for example:
  ```
  // LibreSprite | Copyright (C) 2024 LibreSprite contributors
  // Besprited   | Copyright (C) 2026 Veritaware
  ```
  (use the comment style of the file, e.g. `<!-- ... -->` in XML; see `src/app/app.cpp`, `data/gui.xml` or
  `src/CMakeLists.txt` for examples).
- For a brand-new file, use a plain `// Copyright (C) 2026 Veritaware` line.
- Files that can't carry a header are listed in [`.github/copyright-exempt.txt`](.github/copyright-exempt.txt).

By contributing, you agree that your contribution is released under the project's license,
[GPLv2](LICENSE.txt).

## Branching Model
We use the following branch structure to manage development:
- `trunk`: This is the development branch for Besprited. All new features and bug fixes are merged here first.

Integration of changes from and to LibreSprite is handled in the separate
[veritaware/LibreSprite-integration](https://github.com/veritaware/LibreSprite-integration) repository.

**Contributors from forks:** fork the repository, create a branch in your fork, and open your pull
request against `trunk`.

#### Note for repo contributors:
User branches for features and fixes created *directly* against the `veritaware:Besprited` repository should follow the naming convention:
`username/short-description`, for example: `nidrax/v8-build-fix-windows`.

## Commit Messages and Pull Requests
When committing changes, follow these guidelines:
- Use imperative mood in the subject line (e.g., "Fix bug" instead of "Fixed bug" or "Fixes bug").
- Keep the subject line concise (50 characters or fewer) and provide a more detailed description in the body if necessary.
- If the commit or pull request closes an issue, include a reference to the issue number in the commit message or pull request description (e.g., "Fixes #123").
- If the commit or pull request does not introduce changes to the source code itself (e.g., documentation, helper scripts, issue templates, etc.)
  add the `NO_SW_CHANGE` line to the end of the commit message or pull request description body to skip GitHub build workflows.
- Keep pull requests focused on one issue. Unrelated refactoring or reformatting makes review slower.
- Fill in the pull request template checklist.

## Hacktoberfest
Besprited takes part in [Hacktoberfest](https://hacktoberfest.com/). Contributions are welcome, and
these rules keep review manageable:
- Pick an issue labelled `good first issue` or `help wanted` and **claim it first** (see
  [Finding something to work on](#finding-something-to-work-on)). Pull requests for unclaimed issues may
  be closed.
- Accepted pull requests get the `hacktoberfest-accepted` label.
- Pull requests that are spam or low-effort (e.g. whitespace-only changes, unreviewed AI output, or
  changes that don't build) are labelled `spam` and closed, which also excludes them from Hacktoberfest.

## Usage of AI
We allow the use of LLMs to enhance your work, but at the same time we expect the code to still be clean
and follow good coding practices and be properly marked as AI-co-authored.

> For more extensive guidelines see: [AI_USAGE.md](AI_USAGE.md).

# Contributing

## Branching Model
We use the following branch structure to manage development:
- `trunk`: This is the development branch for Besprited. All new features and bug fixes are merged here first.
- `ls-develop`: This branch should track the latest changes from LibreSprite's `master` branch.
  We periodically merge changes from LibreSprite into our `trunk` branch to stay up-to-date with their developments
  and our repo maintainers use this branch as a base to create any feature or bug fix branches that are shared with LibreSprite's codebase.

### Note for repo contributors:
User branches for features and fixes created *directly* against the `veritaware:Besprited` repository should follow the naming convention:
`username/short-description`, for example: `nidrax/v8-build-fix-windows`.

## Commit Messages and Pull Requests
When committing changes, follow these guidelines:
- Use imperative mood in the subject line (e.g., "Fix bug" instead of "Fixed bug" or "Fixes bug").
- Keep the subject line concise (50 characters or fewer) and provide a more detailed description in the body if necessary.
- If the commit or pull request closes an issue, include a reference to the issue number in the commit message or pull request description (e.g., "Fixes #123").
- If the commit or pull requst does not introduce changes to the source code itself (e.g., documentation, helper scripts, issue templates, etc.)
  add the `NO_SW_CHANGE` line to the end of the commit message or pull request description body to skip GitHub build workflows.
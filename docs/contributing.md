# Contributing

## Workflow

* One change per branch and pull request. `main` only takes changes through a pull request that someone
  other than the author has approved; a direct push to `main` is rejected (`GH013: Repository rule
  violations found`).
* Whoever did not write the change reviews it and runs the tests.
* Resolve every comment thread before merging.
* Merge with **"Create a merge commit"**. The branch is deleted automatically.
* Keep a pushed branch up to date with `git merge origin/main`, not rebase. Commit first, then merge
  `main`: your work is safe in a commit, and `git merge --abort` can always undo the merge.
* CI runs on push only, so it tests the branch, not its merge with `main`. Before approving, check that
  `main` has been merged into the branch and pushed, and that CI passed.

## Commits

* Small commits that can be reviewed one at a time.
* A short summary in the imperative, under 72 characters ("Add the reset hook"), then a blank line and
  the why if it is not obvious.

## Tests

* Run the tests before pushing: `west twister -T tests -p native_sim` (see [Development](development.md)).
* New behaviour gets a test in `tests/`.

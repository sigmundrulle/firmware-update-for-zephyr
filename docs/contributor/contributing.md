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
* A branch that changes behaviour adds or updates tests for it in `tests/`, in the same pull request.
* A bug fix gets a test that fails without the fix.
* Documentation, comments and CI changes don't necessarily need a new test.
* If a change can't be tested on `native_sim` (for example, it needs a real board), say so in the pull
  request and explain why.


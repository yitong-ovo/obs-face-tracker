# Release Guide

## Actions on a fork

GitHub disables inherited workflows when a repository is first forked. Review
`.github/workflows/`, then open the fork's **Actions** tab and choose **I
understand my workflows, go ahead and enable them**. The active workflows can
also be checked with:

```bash
gh workflow list --repo OWNER/obs-face-tracker --all
gh api repos/OWNER/obs-face-tracker/actions/permissions
```

## Branch builds and artifacts

Non-documentation pushes to `main`, pull requests targeting `main`, and manual
`workflow_dispatch` runs build the main platform matrix. Their files appear
under **Artifacts** at the bottom of the Actions run, not on the Releases page.
Documentation-only pushes and pull requests are ignored by this workflow.
Artifacts are temporary and GitHub normally retains them for about 90 days.

Download one with:

```bash
gh run download RUN_ID \
  --repo OWNER/obs-face-tracker \
  --name ARTIFACT_NAME \
  --dir artifacts
```

GitHub wraps each uploaded artifact in an additional ZIP. Extract that wrapper
to reach the `.pkg`, `.zip`, installer, or `.deb` files produced by the job.

## Why the release job is skipped

The release job has this condition:

```yaml
if: startsWith(github.ref, 'refs/tags/')
```

It is expected to be `skipped` for `main`, pull requests, and manually dispatched
branch runs. Only a pushed Git tag satisfies the condition.

## Publish a version

1. Review [Compatibility](compatibility.md) and disclose platforms without
   real-machine testing in the release notes.
2. Update `project(... VERSION ...)` in `CMakeLists.txt`.
3. Update documentation and commit the version change.
4. Create an annotated tag and push it to the writable fork remote:

   ```bash
   git tag -a v0.10.0 -m "v0.10.0"
   git push fork main
   git push fork v0.10.0
   ```

5. Wait for Linux, both macOS architectures, and Windows to pass.
6. The release job downloads the Ubuntu DEB, two macOS architecture packages,
   and Windows packages produced by `Plugin Build`, generates release notes,
   creates the GitHub Release, and attaches those files.

The separate Docker workflow builds Fedora 41/42/43 RPM artifacts. The current
automatic release job does not aggregate those RPMs; attach them manually or
extend the release workflow if Fedora packages must be part of the release.

Do not use `origin` when it points to the read-only upstream repository.

## macOS signing and notarization

Without repository secrets, CI produces unsigned development packages. Signing
and notarization require the complete secret sets referenced in
`.github/workflows/main.yml`, including application and installer identities,
the certificate, and Apple notarization credentials. The workflow skips
notarization when notarization credentials are incomplete.

Unsigned artifacts can be used for testing but should be described clearly in
release notes. Do not claim notarization when the signing steps were skipped.

## Release notes checklist

- State the OBS SDK used by CI.
- State the real-machine test matrix.
- Disclose that the new Hybrid code was developed with GPT-5.6 assistance.
- List model versions and upstream licenses.
- Mention unsigned/not-notarized macOS packages when applicable.
- Link to installation, properties, troubleshooting, and compatibility docs.

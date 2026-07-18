# Compatibility and Test Status

## Read this first

A green GitHub Actions job means that the plugin compiled, was packaged, and
passed the checks implemented by that job. It does not mean that the package
has been opened in OBS and tested with a real camera on that platform.

The new Hybrid detector, OpenCV integration, debug visualization, packaging,
Chinese localization, and related documentation in this fork were developed
with assistance from GPT-5.6. The implementation has been reviewed during
development and manually exercised on the platform listed below, but it still
requires broader human review and real-device testing.

## Current matrix

<!-- markdownlint-disable MD013 -->

| Platform | CI status | Real-machine status | Confidence |
|----------|-----------|---------------------|------------|
| macOS arm64, Apple Silicon | Builds and packages with the OBS 30 SDK | Tested on Apple M4, macOS 15.7.4, OBS 32.0.1 | Primary tested platform |
| macOS x86_64, Intel | Builds and packages with the OBS 30 SDK | Not tested on an Intel Mac | Experimental |
| Windows x64 | Builds and packages with the OBS 30 SDK; OpenCV runtime is included | Not tested on a Windows machine | Experimental |
| Ubuntu 22.04 x86_64 | Builds, packages, installs, and checks shared-library resolution in CI | Not tested in an interactive OBS session | Experimental |
| Fedora 41/42/43 x86_64 | Separate Docker workflow builds RPM packages | Not tested in an interactive OBS session | Experimental |

<!-- markdownlint-enable MD013 -->

OBS 32.0.1 on Apple Silicon is the only current real-machine validation. CI
uses the OBS 30 development environment to preserve the upstream build matrix.
Do not interpret either statement as a guarantee for every OBS 30 or OBS 32
patch release.

## What has been manually checked

- The plugin bundle loads in OBS 32.0.1 on Apple Silicon.
- The Face Tracker filter can be added to a video source.
- YuNet and NanoDet load from the packaged resources.
- Hybrid detection receives video frames and starts correlation trackers.
- Simplified Chinese localization loads.
- Debug colors and line styles are visible in the preview.

## What still needs testing

- Windows installation, DLL loading, camera input, and long-running stability.
- Intel macOS installation, signing behavior, and runtime performance.
- Linux desktop installation and interactive OBS use.
- Different OBS versions, capture formats, camera drivers, and PTZ hardware.
- Multi-person target choice in varied real scenes.
- Long recordings and memory/CPU behavior.

When reporting a problem, include the operating system, CPU architecture, OBS
version, package filename, detector choice, video resolution/format, and the OBS
log. See [Troubleshooting](troubleshooting.md).

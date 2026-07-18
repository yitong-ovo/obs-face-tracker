# Installation and Configuration

## Download a release

Open the repository's **Releases** page and download the package matching your
operating system and CPU. Packages created from a version tag contain the
plugin, translations, YuNet, NanoDet, dlib models, and the corresponding model
license files.

## macOS

### ZIP package

1. Quit OBS completely.
2. Extract the ZIP archive.
3. Copy `obs-face-tracker.plugin` to
   `~/Library/Application Support/obs-studio/plugins/`.
4. Start OBS.

Apple Silicon OBS requires the bundle layout below. Do not move only the binary
out of the bundle.

```text
obs-face-tracker.plugin/
└── Contents/
    ├── MacOS/obs-face-tracker
    ├── lib/
    └── Resources/
```

The `.pkg` release asset performs the installation automatically. Unsigned
development builds may require an ad-hoc signature:

```bash
codesign --force --deep --sign - \
  "$HOME/Library/Application Support/obs-studio/plugins/obs-face-tracker.plugin"
```

## Windows

1. Quit OBS.
2. Run the Windows installer, or extract the ZIP into the OBS installation
   directory while preserving `obs-plugins` and `data`.
3. Start OBS.

The ZIP includes the OpenCV runtime DLLs needed by the Hybrid detector.

## Linux

Install the `.deb` package with its dependencies:

```bash
sudo apt install ./obs-face-tracker-*.deb
```

## Configure the filter

1. Right-click a camera or capture source in OBS and select **Filters**.
2. Under **Effect Filters**, click **+** and choose **Face Tracker**.
3. Open **Face detection options**.
4. Select a detector:
   - **Hybrid wide-angle detection**: recommended for wide shots and small
     faces. YuNet detects faces and NanoDet supplies a person-based fallback.
   - **dlib HOG**: fast option for close, frontal faces.
   - **dlib CNN**: slower legacy deep detector.
5. Release packages fill the model paths automatically. If a path is empty,
   point it to the matching file under the plugin's `Resources` or `data`
   directory.

## Recommended Hybrid settings

- Start with **Scale image** at `2`. Use `1` when faces are very small; use a
  larger value to reduce CPU usage when subjects are close.
- Use detector crop values to exclude screens, posters, or audience areas that
  should never become tracking targets.
- Leave landmark detection off unless you specifically need landmark-based
  framing.
- Enable **Show face detection results** while tuning, then disable it for
  production output.

## Debug box legend

| Color and style | Meaning |
|-----------------|---------|
| Yellow solid | YuNet face |
| Blue dashed | NanoDet person |
| Green dotted | Face estimated from a person |
| Red dashed | dlib HOG face |
| Orange dotted | dlib CNN face |
| Bright green solid | Active correlation tracker |

## Build from source

Initialize dependencies and download verified models:

```bash
git submodule update --init --recursive
bash ci/download-dlib-models.sh
bash ci/download-models.sh
```

Install OpenCV development files before configuring CMake:

```bash
# macOS
brew install openblas opencv

# Ubuntu/Debian
sudo apt install libopenblas-dev libopencv-dev
```

Configure with the libobs and frontend API paths supplied by your OBS SDK, then
build and install:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo <OBS SDK options>
cmake --build build --parallel
cmake --install build --prefix release
```

## Create a release

The `Plugin Build` workflow runs for pushes and pull requests. To publish a
release, update the project version, commit it, then push a version tag:

```bash
git tag v0.10.0
git push origin main v0.10.0
```

After Linux, macOS, and Windows jobs succeed, GitHub Actions creates the Release
and uploads all generated packages. macOS signing and notarization are used
when the documented repository secrets are configured; otherwise the workflow
still produces an unsigned development package.

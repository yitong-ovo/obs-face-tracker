# Troubleshooting

## Collect the OBS log

Reproduce the issue once, then open **Help > Log Files > View Current Log**.
Useful default locations include:

```text
macOS:  ~/Library/Application Support/obs-studio/logs/
Windows: %APPDATA%\obs-studio\logs\
Linux:  ~/.config/obs-studio/logs/
```

Search for:

```text
obs-face-tracker
hybrid:
dlopen
model not found
failed to load
compiled with newer libobs
```

## The plugin is missing from OBS

1. Quit OBS completely, including background processes, then restart it.
2. Confirm that the package matches the CPU architecture.
3. On Apple Silicon, keep the complete bundle in one of these locations:

   ```text
   ZIP/per-user: ~/Library/Application Support/obs-studio/plugins/obs-face-tracker.plugin
   PKG/system:   /Library/Application Support/obs-studio/plugins/obs-face-tracker.plugin
   ```

4. Do not copy only `Contents/MacOS/obs-face-tracker` out of the bundle.
5. On Windows, preserve the package's `obs-plugins` and `data` directories and
   keep `opencv_world4130.dll` beside the plugin binary.

## The module is found but does not load

### Qt symbol error on macOS

An error such as `Symbol not found: qt_...` means the plugin was linked against
a different Qt build from OBS. Build against the Qt supplied by the matching OBS
SDK, or build with `-DWITH_DOCK=OFF` when the dock is not required.

### Newer libobs error

```text
compiled with newer libobs
```

The plugin was built with newer OBS headers than the installed OBS. Install a
matching/newer OBS version or rebuild against the installed version's SDK.

### Missing OpenCV library

On macOS, packaged Homebrew libraries belong in
`Contents/lib`. On Windows, `opencv_world4130.dll` belongs beside the plugin
DLL. Linux packages rely on distribution OpenCV packages.

### Invalid macOS signature

For an unsigned per-user ZIP development build:

```bash
codesign --force --deep --sign - \
  "$HOME/Library/Application Support/obs-studio/plugins/obs-face-tracker.plugin"
codesign --verify --deep --strict --verbose=4 \
  "$HOME/Library/Application Support/obs-studio/plugins/obs-face-tracker.plugin"
```

For a PKG installation, use the system path shown above and appropriate
permissions.

## A Hybrid model does not load

Release packages contain both models and normally select them automatically.
Expected files are:

```text
hybrid/yunet/face_detection_yunet_2023mar.onnx
hybrid/nanodet/nanodet-plus-m_416.onnx
```

Run `bash ci/download-models.sh` in a source checkout to download and verify the
official files. The script rejects files with the wrong SHA-256.

## Hybrid does not detect a distant face

1. Enable **Show face detection results**.
2. Set **Scale image** to `1` so detection uses the full input resolution.
3. Remove or reduce detector crop values.
4. Check whether a blue dashed person box appears.
5. Improve lighting and keep enough of the body visible for the NanoDet fallback.
6. Confirm that the input itself contains enough pixels for the face.

A blue dashed box is a person detection for visualization. A yellow face or
green estimated-face box is used to initialize tracking.

## CPU usage is too high

- Increase **Scale image** from `1` to `2` or higher.
- Crop irrelevant detector regions.
- Avoid dlib CNN unless it provides a measurable accuracy benefit.
- Reduce a 4K source before applying the filter.
- Disable debug overlays after tuning.

## The frame follows the wrong person

The current selector prefers a YuNet face and otherwise uses a NanoDet-derived
face estimate. Sophisticated persistent identity selection is not yet
implemented. Crop irrelevant regions and reset tracking when the intended
subject is clearly visible.

## Debug colors look wrong

Use the legend shown in the filter properties and in
[Hybrid Model Setup](hybrid-models.md#visualization-guide). Restart OBS after
replacing a plugin binary; OBS cannot reload a module already loaded in the
current process.

## Reporting an issue

Include:

- OBS version and full current log.
- Operating system and CPU architecture.
- Exact package or commit.
- Detector and model paths.
- Source resolution and capture format.
- Steps to reproduce and whether the issue occurs with dlib HOG and Hybrid.

Windows, Intel macOS, and interactive Linux operation have not yet received
real-machine testing in this fork. Reports from those platforms are especially
useful.

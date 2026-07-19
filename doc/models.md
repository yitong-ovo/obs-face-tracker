# Model Files: Purpose, Source, and Preparation

## Detection is not identification

This plugin detects faces or people and tracks their position in a video frame.
It does not create face embeddings, compare faces with a database, identify a
named person, or store biometric identities. In this documentation,
"recognition" in an existing UI label means object detection, not identity
recognition.

## Model summary

- `frontal_face_detector.dat`: dlib's built-in HOG face detector; included.
- `mmod_human_face_detector.dat`: dlib CNN face detector from
  `davisking/dlib-models`; included.
- `shape_predictor_5_face_landmarks.dat`: five-point predictor from
  `davisking/dlib-models`; included.
- `shape_predictor_68_face_landmarks.dat`: restricted 68-point predictor from
  `davisking/dlib-models`; not included.
- `shape_predictor_68_face_landmarks_GTX.dat`: restricted faster 68-point
  variant from `davisking/dlib-models`; not included.
- `face_detection_yunet_2023mar.onnx`: YuNet face detector from OpenCV Zoo;
  included.
- `nanodet-plus-m_416.onnx`: person detector from an official NanoDet release;
  included.

The detector models locate a face or person. Landmark models locate geometric
points on an already detected face; they do not identify the person either.

## Prepare the default models

From the repository root, initialize the dlib submodule and run both download
scripts:

```bash
git submodule update --init --recursive
DESTDIR=./ bash ci/download-dlib-models.sh
bash ci/download-models.sh
```

The scripts place the files under `data/`, where CMake can include them in an
installed plugin or release package. Official release packages already contain
the default models and their available upstream license files, so end users do
not normally need to run these commands.

## dlib HOG face detector

`frontal_face_detector.dat` is a serialized copy of dlib's built-in frontal
face detector. It is used by the fast, CPU-oriented dlib HOG mode. The project
contains `src/face-detector-dlib-hog-datagen.cpp`, which writes the bytes
returned by `dlib::get_serialized_frontal_faces()` to standard output.

To generate the file from the checked-out dlib version instead of downloading
the pre-generated copy:

```bash
cmake -S . -B build -DENABLE_DATAGEN=ON
cmake --build build --target face-detector-dlib-hog-datagen
mkdir -p data/dlib_hog_model
./build/face-detector-dlib-hog-datagen \
  > data/dlib_hog_model/frontal_face_detector.dat
```

The executable location can differ with the generator, configuration, and
platform. For example, a multi-configuration Windows build may place it in a
`Release` subdirectory.

`ci/download-dlib-models.sh` instead downloads a compressed copy published in
the upstream obs-face-tracker `0.7.0-hogdata` release and expands it to the same
path. The detector originates in dlib; the package copies dlib's Boost Software
License file as `LICENSE-dlib`.

## dlib CNN and landmark models

The dlib script shallow-clones the official
[`davisking/dlib-models`](https://github.com/davisking/dlib-models) repository
and expands models created and published by dlib author Davis King:

- `mmod_human_face_detector.dat` provides the legacy dlib CNN face detector.
- `shape_predictor_5_face_landmarks.dat` provides the default lightweight
  landmark predictor.

Both are included in normal builds. Davis King states that these model files
are released into the public domain; the repository's CC0 1.0 Universal text
is packaged as `LICENSE-dlib-models`.

The two 68-point files are deliberately excluded from default downloads and
release packages because they were trained on the iBUG 300-W dataset. The
upstream notice says that dataset's license excludes commercial use and that
the trained models therefore cannot be used in a commercial product. They can
be downloaded for an appropriate non-commercial use case only after reviewing
the upstream terms:

```bash
DESTDIR=./ bash ci/download-dlib-models.sh --nonfree
```

That option also writes the upstream model notice to
`data/LICENSE-shape_predictor_68_face_landmarks`. Do not assume the plugin's
GPLv2 license or the general `dlib-models` license removes the model-specific
dataset restrictions.

The dlib download script currently follows the current head of the upstream
model repository and does not pin or verify SHA-256 values. Review and checksum
those inputs separately when a reproducible or audited build is required.

## YuNet face detector

The Hybrid mode uses the model published by OpenCV Zoo as
`face_detection_yunet_2023mar.onnx`, pinned to OpenCV Zoo commit
`47534e27c9851bb1128ccc0102f1145e27f23f98`. YuNet detects face rectangles
directly and is preferred over a body-derived estimate.

- Upstream: [OpenCV Zoo YuNet](https://github.com/opencv/opencv_zoo/tree/47534e27c9851bb1128ccc0102f1145e27f23f98/models/face_detection_yunet)
- License: MIT, copied into packages as `LICENSE-YuNet`
- SHA-256: `8f2383e4dd3cfbb4553ea8718107fc0423210dc964f9f4280604804ed2552fa4`
- Installed path: `data/hybrid/yunet/face_detection_yunet_2023mar.onnx`

## NanoDet person detector

The Hybrid mode uses `nanodet-plus-m_416.onnx`, published by the NanoDet
project in its official `v1.0.0-alpha-1` release. It detects people, not faces.
When YuNet does not find a matching face, the plugin estimates a face region
near the top of a NanoDet person box. The full person box is not used to
initialize the face tracker.

- Upstream: [NanoDet v1.0.0-alpha-1](https://github.com/RangiLyu/nanodet/releases/tag/v1.0.0-alpha-1)
- License: Apache License 2.0, copied into packages as `LICENSE-NanoDet`
- SHA-256: `59d2f166088889c902f523bf08079391993491324f0d84847e3c4016a8f7cc3d`
- Installed path: `data/hybrid/nanodet/nanodet-plus-m_416.onnx`

`ci/download-models.sh` uses fixed URLs for both Hybrid models, verifies these
checksums before installation, and downloads their license texts. A checksum
mismatch stops the script instead of installing a different file.

## Custom or manually downloaded models

Model paths can be selected in the filter's face detection settings. Use the
exact model format expected by that setting; a similarly named ONNX or dlib
file is not necessarily compatible with the plugin's decoder. See
[Hybrid detector model setup](hybrid-models.md) for runtime behavior and debug
visualization, and [Build from source](build-from-source.md) for platform build
requirements.

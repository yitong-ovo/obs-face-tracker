# Hybrid Detector Model Setup

The Hybrid detector combines YuNet (face detection) and NanoDet-Plus-m (body detection) for improved tracking reliability, especially with wide-angle sources.

## License Summary

| Model | License | Download size |
|-------|---------|---------------|
| YuNet | MIT | ~230 KB |
| NanoDet-Plus-m | Apache 2.0 | ~4.7 MB |

The models are downloaded as separate data files and retain their upstream
licenses. Release packages include `LICENSE-YuNet` and `LICENSE-NanoDet`.

## Model Directory Structure

```
data/hybrid/
├── yunet/
│   └── face_detection_yunet_2023mar.onnx
└── nanodet/
    └── nanodet-plus-m_416.onnx
```

## Step 1: Download YuNet (automatic)

The YuNet model can be downloaded automatically:

```bash
cd obs-face-tracker
bash ci/download-models.sh
```

Or manually:

```bash
mkdir -p data/hybrid/yunet
curl -L -o data/hybrid/yunet/face_detection_yunet_2023mar.onnx \
  https://media.githubusercontent.com/media/opencv/opencv_zoo/47534e27c9851bb1128ccc0102f1145e27f23f98/models/face_detection_yunet/face_detection_yunet_2023mar.onnx
```

## Step 2: Download NanoDet-Plus-m ONNX Model

Use the official NanoDet release asset (Apache 2.0):

```bash
mkdir -p data/hybrid/nanodet
curl -L -o data/hybrid/nanodet/nanodet-plus-m_416.onnx \
  https://github.com/RangiLyu/nanodet/releases/download/v1.0.0-alpha-1/nanodet-plus-m_416.onnx
```

The plugin decodes the official `[1,3598,112]` output, including DFL box decoding
and person-class NMS. A custom postprocessed ONNX export is not required.

## Step 3: Install and Use in OBS

After building and installing the plugin:

1. Restart OBS Studio.
2. Add a Face Tracker filter or source.
3. Open **Face detection options**.
4. Set **Detector** to **Hybrid wide-angle detection (YuNet face + NanoDet person)**.
5. Release packages select the bundled models automatically. For a local
   build, select `data/hybrid/yunet/face_detection_yunet_2023mar.onnx` and
   `data/hybrid/nanodet/nanodet-plus-m_416.onnx` if the paths are empty.

## How It Works

```
Video Frame
  ├── YuNet (320×320) ─── face rectangles ──────┐
  │                                               ├── Merge ──→ Trackers
  └── NanoDet (416×416) ─── person rectangles ───┘
                              └── estimate face position
```

- **YuNet** detects faces directly (accurate, used when face is visible)
- **NanoDet** detects full human bodies, then estimates face position from the body box
  (reliable when face is too small for face-only detection)
- If YuNet finds faces, its real face boxes are preferred over body estimates.
- The models run sequentially in the detector thread every ~2 seconds.
- Between detections, dlib's correlation tracker follows the target frame-by-frame

## Visualization Guide

In OBS preview with "Show face detection results" enabled:

| Color | Pattern | Meaning |
|-------|---------|---------|
| Red | Dashed | dlib HOG face detection |
| Orange | Dotted | dlib CNN face detection |
| **Yellow** | **Solid** | **YuNet face detection** |
| **Blue** | **Dashed** | **NanoDet body detection** (original person box) |
| **Green** | **Dotted** | **Estimated face position from body** |
| Bright Green | Solid thick | Correlation tracker (tracking) |

The estimated face box (green dotted) appears when NanoDet finds a person and
YuNet does not provide a more accurate face candidate.

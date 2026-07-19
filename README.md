# Face Tracker Plugin for OBS Studio

## Introduction

This plugin tracks a person in an OBS video source and automatically adjusts the
crop or a PTZ camera to keep the subject framed.

This plugin employs [dlib](http://dlib.net/) for object tracking. Face detection
can use dlib or a wide-angle hybrid of YuNet face detection and NanoDet person
detection.
The frame of the source is periodically taken to face detection algorithm.
Once a face is found, the face is tracked.
Based on the location and size of the tracked face, the frame is cropped.

> [!IMPORTANT]
> The new Hybrid detector, OpenCV integration, debug visualization, packaging,
> Chinese localization, and related documentation were developed with assistance
> from GPT-5.6. This code has been manually exercised on an Apple Silicon Mac
> with OBS 32.0.1. Windows x64, Intel macOS, and Linux packages currently pass
> CI builds, but have not been tested on real machines by the maintainer of this
> fork. Treat those packages as experimental and report logs with any issue.

See [Compatibility and test status](doc/compatibility.md) for the exact support
and validation matrix.

## Documentation

- [Installation and configuration](doc/install-and-configure.md)
- [Face Tracker properties](doc/properties.md)
- [Face Tracker PTZ properties](doc/properties-ptz.md)
- [Model files: purpose, source, licenses, and checksums](doc/models.md)
- [Hybrid models and visualization](doc/hybrid-models.md)
- [Troubleshooting](doc/troubleshooting.md)
- [Build from source](doc/build-from-source.md)
- [Release guide](doc/release-guide.md)
- [Compatibility and test status](doc/compatibility.md)
- [简体中文文档](doc/zh-CN/README.md)

## Usage

For several use cases, total 3 methods are provided.

### Face Tracker Source

The face tracker is implemented as a source. It creates another source that
tracks and zooms into a face.

1. Click the add button on the source list.
2. Add `Face Tracker`.
3. Scroll to the bottom and set `Source` property.

See [Properties](doc/properties.md) for the description of each property.

### Face Tracker Filter

The face tracker is implemented as an effect filter for an existing video
source.

1. Open filters for a source on OBS Studio.
2. Click the add button on `Effect Filters`.
3. Add `Face Tracker`.

See [Properties](doc/properties.md) for the description of each property.

### Face Tracker PTZ

Experimental version of PTZ control is provided as an video filter.

1. Open filters for a source on OBS Studio,
2. Click the add button on `Audio/Video Filters`.
3. Add `Face Tracker PTZ`.

See [Properties](doc/properties-ptz.md) for the description of each property.

See [Limitations](https://github.com/norihiro/obs-face-tracker/wiki/PTZ-Limitation)
for current limitations of PTZ control feature.

## Hybrid detector quick start

Release packages include the YuNet and NanoDet models. Add the **Face Tracker**
filter, open **Face detection options**, and choose **Hybrid wide-angle
detection (YuNet face + NanoDet person)**. See
[Installation and configuration](doc/install-and-configure.md) for complete
platform instructions and recommended settings.

## Builds and releases

GitHub Actions builds Ubuntu x86_64, macOS arm64, macOS x86_64, and Windows x64
packages in the main workflow. A separate Docker workflow builds Fedora RPMs.
A successful CI build proves that the package compiled and was assembled; it
does not prove real-device compatibility. Pushes to `main` that change non-doc
files, pull requests targeting `main`, and manual runs create temporary Actions
artifacts. A version tag creates a GitHub Release after the main platform jobs
pass. See the [Release guide](doc/release-guide.md).

## Known issues

This plugin is heavily under development. These issues are under investigation:

- Memory usage is gradually increasing when continuously detecting faces.
- It consumes a lot of CPU resource.
- The frame sometimes vibrates because the face detection results vibrates.

## License

This plugin is licensed under GPLv2.

## Sponsor

- [Jimcom USA](https://www.jimcom.us/?ref=2) - a company for live-streaming and
  recording professionals.
  Development of PTZ camera control is supported by Jimcom.
  Jimcom provides a 20% discount for its broadcast-quality, network-connected
  PTZ cameras and free shipping in the USA. Enter coupon code **FACETRACK20**.

## Acknowledgments

- [dlib](http://dlib.net/) - [GitHub repository](https://github.com/davisking/dlib)
- [OpenCV](https://opencv.org/) and
  [OpenCV Zoo](https://github.com/opencv/opencv_zoo)
- [NanoDet](https://github.com/RangiLyu/nanodet)
- [obs-ptz](https://github.com/glikely/obs-ptz) - optional PTZ control backend.
- [OBS Project](https://obsproject.com/)
- GPT-5.6 assisted with the new Hybrid implementation, packaging, localization,
  and documentation in this fork. The resulting code remains subject to review
  and real-platform testing.

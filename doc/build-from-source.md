# Build from Source

## Status and scope

The current code builds in GitHub Actions for Linux x86_64, macOS arm64, macOS
x86_64, and Windows x64. Only Apple Silicon with OBS 32.0.1 has been exercised
on a real machine. See [Compatibility](compatibility.md).

The new Hybrid/OpenCV implementation was developed with assistance from
GPT-5.6 and should be reviewed and tested for the target platform before it is
distributed as production software.

## Dependencies

- A C++20 compiler and CMake.
- An OBS development environment containing `libobs`, `obs-frontend-api`, and
  the matching Qt build when `WITH_DOCK=ON`.
- Git submodules: dlib and libvisca.
- OpenBLAS.
- OpenCV components: core, imgproc, dnn, and objdetect.

Initialize the checkout:

```bash
git submodule update --init --recursive
bash ci/download-dlib-models.sh
bash ci/download-models.sh
```

The model scripts place data under `data/`, which CMake includes in packages.

## macOS

Install dependencies:

```bash
brew install cmake ninja openblas opencv
export OPENBLAS_HOME="$(brew --prefix openblas)"
```

Configure with the CMake package directories supplied by the OBS SDK:

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_OSX_ARCHITECTURES=arm64 \
  -Dlibobs_DIR=/path/to/obs-sdk/libobs/cmake \
  -Dobs-frontend-api_DIR=/path/to/obs-sdk/frontend-api/cmake
cmake --build build --parallel
cmake --install build --prefix release
```

Use `x86_64` for an Intel build. The dock links Qt; use the exact Qt version
provided by the OBS SDK. `-DWITH_DOCK=OFF` removes the optional dock and its Qt
runtime dependency.

After installing, package Homebrew OpenCV dependencies into
`obs-face-tracker.plugin/Contents/lib` and rewrite their install names. The
reference implementation is the macOS job in `.github/workflows/main.yml`.

## Ubuntu/Debian

```bash
sudo apt update
sudo apt install cmake build-essential libopenblas-dev libopencv-dev
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  <options supplied by the OBS SDK>
cmake --build build --parallel
cmake --build build --target package
```

## Windows

The reference CI downloads the official OpenCV 4.13.0 Windows SDK, discovers
`OpenCVConfig.cmake`, builds with Visual Studio 2022, and copies
`opencv_world4130.dll` into the plugin binary directory. See the Windows job in
`.github/workflows/main.yml` for an executable example.

## Match the OBS version

OBS rejects modules compiled with a newer libobs API. Build with headers and
libraries matching the oldest OBS version you intend to support. A successful
compile against one SDK does not guarantee compatibility with every later or
earlier OBS build.

## Verification

Run before committing:

```bash
clang-format -i -fallback-style=none $(git ls-files 'src/*' 'ui/*' | grep '\.[ch]')
git diff --check
cmake --build build --parallel
actionlint -shellcheck= .github/workflows/main.yml
```

Then install the package on a real machine, restart OBS, add the filter, load
both Hybrid models, and inspect the OBS log. CI alone is not runtime testing.

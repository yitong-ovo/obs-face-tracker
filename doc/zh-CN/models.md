# 模型文件：用途、来源与准备方式

## 本插件不识别人物身份

本插件只检测画面中的人脸或人体，并持续跟踪其位置。它不会生成人脸特征向量、
与人员数据库比对、判断画面中是谁，也不会保存生物身份信息。现有界面中的
“识别”表示目标检测，不表示人物身份识别。

## 模型一览

- `frontal_face_detector.dat`：dlib 内置 HOG 人脸检测器；默认打包。
- `mmod_human_face_detector.dat`：来自 `davisking/dlib-models` 的 dlib CNN
  人脸检测器；默认打包。
- `shape_predictor_5_face_landmarks.dat`：来自 `davisking/dlib-models` 的
  5 点关键点模型；默认打包。
- `shape_predictor_68_face_landmarks.dat`：受限的 68 点关键点模型；不打包。
- `shape_predictor_68_face_landmarks_GTX.dat`：受限且较快的 68 点版本；
  不打包。
- `face_detection_yunet_2023mar.onnx`：OpenCV Zoo 发布的 YuNet 人脸检测器；
  默认打包。
- `nanodet-plus-m_416.onnx`：NanoDet 官方 Release 发布的人体检测器；
  默认打包。

检测模型负责找到人脸或人体。关键点模型只在已找到的人脸上定位几何特征点，
同样不能判断人物身份。

## 准备默认模型

在仓库根目录初始化 dlib submodule，然后运行两个下载脚本：

```bash
git submodule update --init --recursive
DESTDIR=./ bash ci/download-dlib-models.sh
bash ci/download-models.sh
```

文件会保存到 `data/`，CMake 安装或打包时会包含这些模型。正式 Release 已经
包含默认模型和可获得的上游许可证文件，普通用户通常不需要自己执行脚本。

## dlib HOG 人脸检测器

`frontal_face_detector.dat` 是 dlib 内置正面人脸检测器的序列化副本，用于速度
较快、主要依赖 CPU 的 dlib HOG 模式。仓库中的
`src/face-detector-dlib-hog-datagen.cpp` 会将
`dlib::get_serialized_frontal_faces()` 返回的数据写入标准输出。

如果希望根据当前 checkout 的 dlib 版本生成，而不是下载预先生成的文件：

```bash
cmake -S . -B build -DENABLE_DATAGEN=ON
cmake --build build --target face-detector-dlib-hog-datagen
mkdir -p data/dlib_hog_model
./build/face-detector-dlib-hog-datagen \
  > data/dlib_hog_model/frontal_face_detector.dat
```

可执行文件的实际位置取决于平台、CMake generator 和构建配置。例如 Windows
多配置构建可能将它放在 `Release` 子目录。

`ci/download-dlib-models.sh` 采用另一种方式：从上游 obs-face-tracker 的
`0.7.0-hogdata` Release 下载压缩副本，再解压到同一路径。检测器源自 dlib；
打包时会将 dlib 的 Boost Software License 复制为 `LICENSE-dlib`。

## dlib CNN 与关键点模型

dlib 脚本会浅克隆官方
[`davisking/dlib-models`](https://github.com/davisking/dlib-models) 仓库，
并解压 dlib 作者 Davis King 制作和发布的模型：

- `mmod_human_face_detector.dat`：旧版 dlib CNN 人脸检测器。
- `shape_predictor_5_face_landmarks.dat`：默认的轻量 5 点关键点模型。

普通构建会包含这两个文件。Davis King 声明这些模型文件已发布到 public
domain；仓库中的 CC0 1.0 Universal 文本会打包为 `LICENSE-dlib-models`。

两个 68 点模型默认不会下载，也不会进入 Release 包，因为它们使用 iBUG 300-W
数据集训练。上游说明明确指出该数据集许可证排除商业使用，因此训练所得模型
不能用于商业产品。确认自己的非商业用途符合上游条款后，才可显式下载：

```bash
DESTDIR=./ bash ci/download-dlib-models.sh --nonfree
```

该选项还会生成 `data/LICENSE-shape_predictor_68_face_landmarks`，其中包含上游
模型说明。不能认为插件的 GPLv2 或通用 `dlib-models` 许可证会消除这些模型
特定的训练数据限制。

dlib 下载脚本目前跟随上游模型仓库的当前 HEAD，没有固定 commit，也不会校验
SHA-256。需要可复现或审计构建时，应另行固定并校验这些输入。

## YuNet 人脸检测器

Hybrid 模式使用 OpenCV Zoo 发布的 `face_detection_yunet_2023mar.onnx`，固定
到 OpenCV Zoo commit `47534e27c9851bb1128ccc0102f1145e27f23f98`。YuNet 直接
检测人脸框，优先级高于根据人体估算的脸框。

- 上游：[OpenCV Zoo YuNet](https://github.com/opencv/opencv_zoo/tree/47534e27c9851bb1128ccc0102f1145e27f23f98/models/face_detection_yunet)
- 许可证：MIT，包内文件名为 `LICENSE-YuNet`
- SHA-256：`8f2383e4dd3cfbb4553ea8718107fc0423210dc964f9f4280604804ed2552fa4`
- 安装路径：`data/hybrid/yunet/face_detection_yunet_2023mar.onnx`

## NanoDet 人体检测器

Hybrid 模式使用 NanoDet 项目在官方 `v1.0.0-alpha-1` Release 发布的
`nanodet-plus-m_416.onnx`。它检测人体，不直接检测人脸。YuNet 没有找到匹配的
人脸时，插件才会根据 NanoDet 人体框顶部估算脸框；完整人体框不会用于初始化
人脸 tracker。

- 上游：[NanoDet v1.0.0-alpha-1](https://github.com/RangiLyu/nanodet/releases/tag/v1.0.0-alpha-1)
- 许可证：Apache License 2.0，包内文件名为 `LICENSE-NanoDet`
- SHA-256：`59d2f166088889c902f523bf08079391993491324f0d84847e3c4016a8f7cc3d`
- 安装路径：`data/hybrid/nanodet/nanodet-plus-m_416.onnx`

`ci/download-models.sh` 对两个 Hybrid 模型使用固定 URL，安装前校验上述摘要，
并下载许可证文本。校验失败时脚本会停止，不会安装内容不同的文件。

## 自定义或手工下载模型

可在滤镜的人脸检测设置中选择模型路径。必须使用对应设置所要求的精确模型
格式；名称相似的 ONNX 或 dlib 文件不一定兼容插件的解码方式。运行逻辑和
调试框说明见[英文 Hybrid 模型说明](../hybrid-models.md)，平台构建要求见
[英文源码构建说明](../build-from-source.md)。

# 常见问题与排障

## 获取 OBS 日志

先复现一次问题，再使用 OBS 的 **帮助 > 日志文件 > 查看当前日志**。

常见目录：

```text
macOS:  ~/Library/Application Support/obs-studio/logs/
Windows: %APPDATA%\obs-studio\logs\
Linux:  ~/.config/obs-studio/logs/
```

建议搜索：

```text
obs-face-tracker
hybrid:
dlopen
model not found
failed to load
compiled with newer libobs
```

## OBS 中找不到插件

- 完全退出并重启 OBS。
- 确认包的 CPU 架构正确。
- Apple Silicon 必须保留完整 `.plugin` bundle。
- ZIP 通常安装在用户目录，PKG 安装在
  `/Library/Application Support/obs-studio/plugins/` 系统目录。
- Windows 必须保留插件 DLL、OpenCV DLL 和 `data` 目录。

## macOS 加载时报 Qt 符号错误

插件链接了与 OBS 不同的 Qt。使用匹配 OBS SDK 的 Qt 重新编译；不需要 Dock
时可以使用 `-DWITH_DOCK=OFF` 移除 Qt 运行时依赖。

## `compiled with newer libobs`

插件使用了比当前 OBS 更新的头文件。升级 OBS，或用匹配当前 OBS 的 SDK 重新
编译。

## 模型不存在或加载失败

Release 包应包含：

```text
hybrid/yunet/face_detection_yunet_2023mar.onnx
hybrid/nanodet/nanodet-plus-m_416.onnx
```

源码目录中可以运行：

```bash
bash ci/download-models.sh
```

脚本会固定版本并验证 SHA-256，不接受损坏或错误的模型文件。

## 广角小脸识别不到

1. 开启识别框。
2. 将检测下采样倍率改成 `1`。
3. 减少检测裁剪。
4. 看是否出现蓝色人体框。
5. 改善光线，并尽量让人体上半身或全身可见。

## CPU 占用高

- 将检测下采样倍率从 `1` 调到 `2` 或更大。
- 裁掉不需要检测的区域。
- 避免使用 dlib CNN。
- 先降低 4K 来源分辨率再应用滤镜。
- 调整完成后关闭调试框。

## 跟错人

当前逻辑优先选择 YuNet 人脸，否则选择 NanoDet 人体估算脸。尚未实现复杂的
长期身份锁定。可以裁掉无关区域，在目标人物清晰可见时点击 **重新锁定目标**。

## 平台特别说明

当前仅在 Apple Silicon + OBS 32.0.1 上进行过人工实机验证。Windows、Intel
Mac 和 Linux 虽然 CI 构建成功，但没有在对应真实机器上完成 OBS 加载、摄像头
和长时间运行测试。报告这些平台的问题时，请务必附上完整日志、系统版本、CPU
架构、OBS 版本和下载的文件名。

新 Hybrid 代码由 GPT-5.6 协助开发，欢迎人工代码审查和不同平台的实测反馈。

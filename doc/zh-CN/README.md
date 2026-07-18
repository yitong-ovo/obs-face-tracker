# OBS 人脸自动跟踪插件

本插件可以识别人脸并持续跟踪，在 OBS 中自动裁切画面，或控制支持的
PTZ 云台相机。

## 重要说明

> [!IMPORTANT]
> 本 fork 新增的 Hybrid 检测、OpenCV 集成、调试框、打包流程、中文翻译
> 和相关文档由 GPT-5.6 协助开发。代码已经在 Apple M4、macOS 15.7.4、
> OBS 32.0.1 上进行人工运行验证。Windows x64、Intel Mac 和 Linux 当前
> 只有 GitHub CI 编译/打包验证，没有经过对应真实机器上的 OBS、摄像头和
> 长时间运行测试，因此应视为实验性支持。

CI 显示绿色只代表编译和既定检查成功，不代表实际设备兼容性已经得到验证。

## 文档导航

- [安装与配置](install-and-configure.md)
- [参数说明](properties.md)
- [常见问题与排障](troubleshooting.md)
- [英文兼容性与测试状态](../compatibility.md)
- [Hybrid 模型说明](../hybrid-models.md)
- [源码构建](../build-from-source.md)
- [发布流程](../release-guide.md)

## 快速开始

1. 在 Release 或 Actions Artifact 中下载符合系统和 CPU 架构的包。
2. 完全退出 OBS 后安装插件，再重新启动 OBS。
3. 右键摄像头来源，打开 **滤镜**。
4. 在 **效果滤镜** 中添加 **人脸自动跟踪**。
5. 打开 **目标识别与检测范围**。
6. 选择 **广角增强检测（YuNet 人脸 + NanoDet 人体，推荐）**。
7. 调试时开启 **在预览中显示识别框**。

# 安装与配置

## 选择下载文件

正式版本位于仓库的 **Releases** 页面。手动构建、`main` 分支构建和 PR
构建位于对应 Actions run 页面最下方的 **Artifacts** 区域。

| 文件标识 | 适用平台 | 实机验证情况 |
|----------|----------|--------------|
| `macos-arm64` | Apple Silicon Mac | 已在 Apple M4 + OBS 32.0.1 验证 |
| `macos-x86_64` | Intel Mac | CI 成功，未在真实 Intel Mac 上测试 |
| `windows-x64` | 64 位 Windows | CI 成功，未在真实 Windows 上测试 |
| `linux-ubuntu-22.04` | Ubuntu 22.04 x86_64 | CI 安装检查成功，未进行交互式 OBS 测试 |

Artifact 是临时文件，通常保留约 90 天。GitHub 会在构建产物外再包装一层
ZIP，所以下载后要先解压外层 ZIP，才能看到 `.pkg`、安装器、`.deb` 或插件
ZIP。

主工作流在非纯文档的 `main` push、目标为 `main` 的 PR、手动运行和 tag 时
触发。Fedora RPM 由单独的 Docker workflow 构建，不会被当前自动 Release job
汇总。

## macOS

### 使用 PKG

1. 完全退出 OBS。
2. 运行对应架构的 `.pkg`。
3. 启动 OBS。

如果该构建没有配置 Apple 开发者证书，它属于未签名开发包，可能被
Gatekeeper 拦截。

### 使用 ZIP

1. 完全退出 OBS。
2. 解压插件 ZIP。
3. 将完整的 `obs-face-tracker.plugin` 复制到：

   ```text
   ~/Library/Application Support/obs-studio/plugins/
   ```

4. 不要只复制 `Contents/MacOS` 里的二进制。
5. 必要时进行 ad-hoc 签名：

   ```bash
   codesign --force --deep --sign - \
     "$HOME/Library/Application Support/obs-studio/plugins/obs-face-tracker.plugin"
   ```

Apple Silicon OBS 需要完整 bundle：

```text
obs-face-tracker.plugin/
└── Contents/
    ├── MacOS/obs-face-tracker
    ├── lib/
    └── Resources/
```

### 升级与卸载

- ZIP 用户目录：

  ```text
  ~/Library/Application Support/obs-studio/plugins/obs-face-tracker.plugin
  ```

- PKG 系统目录：

  ```text
  /Library/Application Support/obs-studio/plugins/obs-face-tracker.plugin
  ```

- 升级：退出 OBS 后覆盖正确位置的整个 `.plugin` bundle。
- 卸载：退出 OBS 后从对应位置删除整个 `obs-face-tracker.plugin`。

## Windows

1. 完全退出 OBS。
2. 运行 Windows Installer，或将 ZIP 内容按原目录结构放入 OBS 安装目录。
3. 不要拆散 `obs-plugins` 与 `data` 目录。
4. 确认 `opencv_world4130.dll` 与插件 DLL 一起安装。
5. 启动 OBS 并检查日志。

Windows 包已经通过 CI 编译和打包，但目前没有在真实 Windows 机器上完成
OBS 加载、摄像头输入和长期运行测试。

## Linux

Ubuntu/Debian：

```bash
sudo apt install ./obs-face-tracker-*.deb
```

CI 会安装 `.deb` 并检查共享库解析，但目前没有进行交互式 OBS 和摄像头测试。

## 添加滤镜

1. 在 OBS 中右键摄像头或采集卡来源。
2. 选择 **滤镜**。
3. 在 **效果滤镜** 中点击 **+**。
4. 添加 **人脸自动跟踪**。
5. 展开 **目标识别与检测范围**。
6. 选择识别方式：
   - **广角增强检测**：广角、全身、小脸场景推荐。
   - **经典人脸检测**：近景正脸，速度快。
   - **深度人脸检测**：旧版深度检测，通常最慢。

Release 包会自动填写模型路径。路径为空时，手工选择插件 `Resources` 或
`data` 目录下对应模型。

各模型的用途、上游来源、许可证、SHA-256 校验情况，以及“目标检测不等于
人物身份识别”的说明见[模型文件说明](models.md)。

## 推荐起始设置

- **检测下采样倍率**：先使用 `2`。
- 小脸漏检：改为 `1`，使用原始分辨率，CPU 占用会增加。
- CPU 过高且人物较近：尝试大于 `2`。
- 使用检测裁剪排除屏幕、海报、观众区等不应成为目标的区域。
- 调试时开启识别框，正式输出时关闭。

## 调试框

| 颜色和线型 | 含义 |
|------------|------|
| 黄色实线 | YuNet 直接识别的人脸 |
| 蓝色虚线 | NanoDet 识别的人体，仅用于调试显示 |
| 绿色点线 | 根据人体位置估算的人脸 |
| 红色虚线 | dlib HOG 人脸 |
| 橙色点线 | dlib CNN 人脸 |
| 亮绿色实线 | 当前正在持续跟踪的目标 |

蓝色人体框不会直接作为人脸跟踪框。系统优先使用 YuNet 人脸，否则使用
人体顶部估算的人脸位置。

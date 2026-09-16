# klogg 简体中文版

基于 [variar/klogg](https://github.com/variar/klogg) 的简体中文维护分支。
保留原有日志读取、索引、搜索、正则表达式、编码检测和 Hyperscan；只调整界面本地化及其验证、打包流程。

## Windows 下载与运行

1. 打开本仓库的 **Actions → Build zh_CN Windows x64**。
2. 选择成功的构建，下载 **klogg-windows-x64-qt6-zh_CN**。
3. 解压整个压缩包，运行 `klogg.exe`。不要单独移动 EXE。
4. 若要把设置保存在解压目录，运行 `klogg_portable.exe`。

唯一交付配置为 Windows x64、Qt 6.7.3、Release。中文应用翻译和 Qt 标准控件翻译均内置于程序，无需手动复制 `.qm`。

无历史配置时，简体中文系统默认使用简体中文，繁体中文系统使用现有繁体翻译，其他系统使用英文。
已有 `view.language` 设置优先；可在 **文件 → 设置 → 语言** 中切换 English / 中文（简体）。修改后重启 klogg，使全部界面生效。

## 维护与构建

汉化源码位于 `zh-cn` 分支，`master` 保留上游基线。上游来源和许可证见原 [README](README.md)、[COPYING](COPYING) 及 [NOTICE](NOTICE)。

在 Actions 页面点击 **Run workflow** 可手动构建。推送到 `zh-cn` 也会触发同一个 Windows x64 Qt6 工作流，不生成 x86、Linux、macOS 或 ARM 包。

不要求本地安装完整 Qt 或 Visual Studio；本地可执行：

```powershell
python scripts/check_translation.py
python scripts/test_check_translation.py
```

修改用户可见文案时保持英文 source，使用 Qt `tr()` / `translate()`，再运行项目现有的 `lupdate` target，并补齐 `src/app/i18n/zh_CN.ts`。
CI 会重新执行 `lupdate`、翻译检查、Release 编译、测试和部署检查。占位符、助记键、换行、富文本结构发生损坏或存在 unfinished/空翻译时检查失败。

发布产物附带上游许可证。翻译统计、测试证据与已知限制见 [汉化报告](LOCALIZATION_REPORT.zh-CN.md)。

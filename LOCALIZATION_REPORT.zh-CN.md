# klogg 简体中文汉化报告

## 范围与来源

- 上游：`variar/klogg`，基线 `25c7de6d8f6da2ce6a00882e5af70b4f331af4c5`。
- 用户仓库：<https://github.com/d4go/klogg>，汉化分支 `zh-cn`。
- 交付目标：Windows x64、Qt 6.7.3、Release，由 GitHub Actions 构建。
- 不修改日志读取、索引、正则表达式、编码检测、缓存或搜索引擎行为；保留 Hyperscan。

## 修改文件

- `src/app/i18n/zh_CN.ts`：完整审校、术语统一、源码同步。
- `src/settings/include/configuration.h`、`src/settings/src/configuration.cpp`：首次运行语言选择和现有设置优先。
- `src/ui/` 中相关窗口、编码菜单、暂存区和快速查找文案：接入 Qt 翻译，保持英文 source。
- `src/ui/src/highlighterset.cpp`：配置缺少 `name` 时的高亮规则集及颜色标签回退名称接入翻译，保留已有名称和正则表达式。
- `src/crash_handler/src/crashhandler.cpp`、`issuereporter.cpp`：异常提示接入翻译。
- `src/logdata/src/logdataworker.cpp`：仅将行过长错误弹窗文案接入翻译，不改索引逻辑。
- `src/app/CMakeLists.txt`：补上已有繁体中文资源及 Qt 对应资源。
- `tests/ui/translation_test.cpp`、`tests/ui/CMakeLists.txt`：locale、配置持久化和内嵌翻译验证。
- `scripts/check_translation.py`、`scripts/test_check_translation.py`：翻译质量门禁及回归样例。
- `.github/workflows/build-zh-cn-windows.yml` 和 Windows 打包/验证脚本：独立 x64 Qt6 工作流。
- 既有工作流仅增加 `zh-cn` 跳过条件，避免本交付触发其他平台任务。

## 翻译统计

| 指标 | 修改前 | 同步并汉化后 |
|---|---:|---:|
| 有效条目 | 395 | 487 |
| 已完成 | 194 | 487 |
| unfinished | 201 | 0 |
| 空翻译 | 3 | 0 |
| source 与 translation 相同 | 13 | 13 |
| obsolete / vanished | 0 | 0 |

保留原文的条目均已人工核对：品牌名、编码名 Unicode、技术名 Qt/Hyperscan、容量单位、分隔符或参数格式。占位符按出现次数核对，包括 `%1`、`%L1`、`%n`；助记键字母（忽略大小写）、换行及 HTML 标签/属性均通过静态检查。

Qt 6.7.3 `lupdate` 已全量扫描 `src`，移除 8 条过期 source，并补齐 100 条新提取文案。`lrelease` 已生成 487 条完成翻译的 `zh_CN.qm`。最终 QM 由 CI 重新生成并嵌入 EXE，不要求用户安装外部语言文件。

## 修复的硬编码英文

恢复默认快捷键确认、快捷键录入、自动编码选项、编码语言分组、快速查找方向及结束提示、搜索结果切换提示、暂存区转换工具/时间/标签命名、新建高亮规则集、崩溃报告提示及超长日志行错误等。

最终审计补齐配置缺少 `name` 时的两条回退文案：`Highlighters set` → 高亮规则集，`Color label %1` → 颜色标签 %1。已有用户名称原样保留；新建规则使用的 `New Highlighter` 是实际匹配模式，保持原文以避免改变匹配行为。

## 语言行为

- 新配置：简体中文 locale → `zh_CN`；繁体中文 locale → 现有 `zh_TW`；其他 → `en`。
- 已有 `view.language`：尊重用户选择，不覆盖。
- 英文：移除翻译器并恢复英文 source，不依赖不存在的 `qt_en.qm`。
- Qt 翻译缺失不会阻止应用翻译加载。
- **文件 → 设置 → 视图 → 语言** 可切换 English / 中文（简体）；明确提示重启后所有界面完整生效。
- 不替换生产 UI 默认字体，也不打包中文字体。

## 验证状态

- PASS：Qt 6.7.3 `lupdate` 和 `lrelease`。
- PASS：487 条翻译静态检查（0 错误、0 待复核警告）。
- PASS：翻译检查器 7 个正反例回归测试。
- PASS：工作流 YAML / actionlint 和 PowerShell 语法检查。
- PASS：`git diff --check`。
- 已预览：源 `.ui` 设置界面的中文排版（这不等同于完整程序运行验证）。

最终构建的动态验证尚未完成。下表须以同一交付源码对应的 CI 日志或实际运行记录补齐；测试已编写、源界面预览和编译通过均不能代替运行结果。

| 验证项 | 验证方式 | 最终结果 | 证据 |
|---|---|---|---|
| 首次语言选择与已有配置保留 | C++ locale / 配置测试 | 待验证 | 待补 CI 日志 |
| 内嵌应用 QM 与 Qt 标准按钮翻译 | C++ 翻译资源测试 | 待验证 | 待补 CI 日志 |
| 中文主窗口、设置各页、暂存区 | 界面测试截图及实际程序检查 | 待验证 | 待补截图 / 记录 |
| Windows 中文系统首次启动 | 无历史配置的实际程序检查 | 待验证 | 待补运行记录 |
| 中文与英文切换、重启后持久化 | 实际程序检查 | 待验证 | 待补运行记录 |
| 打开日志、搜索、正则表达式及筛选 | C++ 功能测试与运行包测试 | 待验证 | 待补测试日志 |
| 大于 1 GiB 日志读取及搜索 | 运行包测试 | 待验证 | 待补文件规模 / 结果 |
| 实时跟踪与追加日志 | C++ 界面测试及实际程序检查 | 待验证 | 待补测试日志 / 记录 |
| UTF-8、UTF-16 LE/BE、GB18030 与中文 / emoji | 运行包测试及实际程序检查 | 待验证 | 待补测试日志 / 记录 |
| EXE / DLL 均为 x64，部署依赖完整 | PE 架构检查与隔离环境启动 | 待验证 | 待补部署检查日志 |

## 构建与产物

工作流：**Build zh_CN Windows x64**，支持 `workflow_dispatch` 和 `zh-cn` 推送。

预期主产物：`klogg-windows-x64-qt6-zh_CN`，包含 EXE、Qt6 DLL、Windows 平台插件和所需运行库；翻译证据产物为 `klogg-windows-x64-qt6-zh_CN-i18n`。

| 项目 | 最终交付记录 |
|---|---|
| 构建源码提交 | 待最终成功构建后填写 |
| Actions 构建链接 | 待最终成功构建后填写 |
| 目标架构 / 配置 | Windows x64 / Release |
| Qt | 工作流固定为 6.7.3；待最终日志确认 |
| 编译器 | 待填写最终 MSVC 版本 |
| 可执行文件 | `klogg.exe`、`klogg_portable.exe`、`klogg_grep.exe`；待最终部署验证 |
| 下载链接 | 待最终成功构建后填写 |
| 测试证据链接 | 待最终成功构建后填写 |

## 已知限制

本报告将在正式 CI 完成后补充真实构建链接和测试结果。动态验证中的“待验证”不表示通过；最终报告会分别记录自动测试与实际程序检查。

- 内置上游帮助文档、命令行帮助及 GitHub issue 模板保留英文。
- 更新弹窗的应用提示已翻译；远端返回的更新说明保留原文。
- 应用错误提示已接入翻译；部分正则表达式引擎、网络或系统返回的第三方诊断可能仍为英文，不改写其技术内容。
- 已有用户命名、日志内容、查询表达式、路径、编码名和技术名称保持原样。
- 繁体翻译沿用上游内容，没有在本任务中全面审校。

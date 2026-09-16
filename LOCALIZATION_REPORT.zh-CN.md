# klogg 简体中文汉化完成报告

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
- `tests/ui/crawlerwidget_test.cpp`：追加日志、开启与关闭实时跟踪的回归验证。
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

[最终 CI](https://github.com/d4go/klogg/actions/runs/35087578421) **全部成功**，构建版本为 **24.11.0.724**，源码提交为 `51f1be0ff401d3b9b8e23cb093b8377356abb196`。CTest **4/4 通过**；运行包的依赖、全部 EXE / DLL 的 x64 架构、独立启动及日志读取搜索检查全部通过。

CTest 包括单元测试、UI / Follow 测试、i18n 测试和启动冒烟检查；前三组共 **17 个用例、5084 次断言**。

| 测试组 | 用例数 | 断言数 | 最终 CI 结果 |
|---|---|---|---|
| 单元测试 | 4 | 4054 | PASS |
| UI 与 Follow | 9 | 974 | PASS |
| i18n | 4 | 56 | PASS |
| 启动冒烟检查 | 单独的 CTest 项 | 不计入上述断言数 | PASS |

原生窗口的完整操作验证使用 **24.11.0.722 / `577af6d9`**；之后仅补了两条缺失配置名称的翻译及打包测试修正。最终 **24.11.0.724** 已另外从正式 Artifact 下载，在新的便携目录复核首次中文启动、UTF-8 自动识别和中文日志显示，并正常退出。下表区分自动测试与原生窗口检查，不把离屏截图当作手动操作验证。

| 验证项 | 验证方式 | 实测结果 |
|---|---|---|
| 首次语言选择与已有配置保留 | C++ locale / 配置测试 | PASS；覆盖 13 个 locale 输入及已有语言配置 |
| 内嵌应用 QM 与 Qt 标准按钮翻译 | C++ 翻译资源测试 | PASS |
| 中文主窗口、设置各页、暂存区 | C++ 界面测试与离屏截图 | PASS；离屏 emoji 显示限制见下文 |
| 中文主菜单、日志右键、设置通用 / 视图页、Qt 标准按钮 | 本机原生窗口检查 | PASS，中文显示正常 |
| Windows 中文系统首次启动 | 无历史配置的 `klogg_portable.exe` | PASS，默认简体中文 |
| 中文与英文切换、重启后持久化 | 本机原生窗口检查 | PASS；中文 → English 重启后完整英文（含 Encoding 菜单），再切中文并重启后完整中文；配置 `view.language` 依次确认为 `en`、`zh_CN` |
| UTF-8、UTF-16 LE、UTF-16 BE、GB18030 混合日志 | 分别打开 504 行样本 | PASS，四种样本均自动识别正确，中文显示正常 |
| 日志搜索 | 本机 GUI 搜索 `ERROR`、`中文错误` | PASS，分别命中 2 条、1 条 |
| 正则表达式、筛选及日志功能 | C++ 功能测试 | PASS，见对应 CI 测试日志 |
| 大于 1 GiB 日志读取及搜索 | 本机 GUI 打开并搜索 | PASS；文件 1,077,936,183 字节，显示 263169 行，末尾 `ERROR` 命中 1 条；自动检测限制见下文 |
| 实时跟踪与追加日志 | C++ Follow 测试及本机 GUI 检查 | PASS；初始 200 行，开启实时跟踪后追加 25 行，自动滚动至第 225 行 |
| 中文与 emoji 字形 | 本机 GUI 显式选择 UTF-8 | PASS，中文与彩色 😀🚀 正常；字体设置中 `DejaVu Sans Mono` 显示正常 |
| 运行包的多编码命令行搜索 | 最终 CI 独立运行验证，临时配置明确指定编码 MIB；本机另有验证 | PASS；4 种编码各 4 个查询、emoji 样本及大于 1 GiB 文件尾部匹配全部通过 |

命令行 `klogg_grep` 没有 GUI 应用自动检测结果所用的 codec 设置流程，因此运行包验证脚本通过临时配置指定各样本的编码 MIB。这组结果证明指定编码下的读取与搜索正确，不能作为自动编码检测通过的证据；上表四种 504 行样本的自动检测结果来自独立 GUI 实测。

最终源码与生成的 QM 均包含 487 条完成翻译。正式包附带 `VALIDATION.txt`；完整测试日志、同步后的 TS / QM 和 7 张离屏界面截图见验证证据压缩包。

## 构建与产物

工作流：**Build zh_CN Windows x64**，支持 `workflow_dispatch` 和 `zh-cn` 推送。

主产物：`klogg-windows-x64-qt6-zh_CN`，包含 EXE、Qt6 DLL、Windows 平台插件和所需运行库；翻译证据产物为 `klogg-windows-x64-qt6-zh_CN-i18n`。Release 下载包基于已通过验证的正式 Artifact，另附最终中文说明和报告，EXE / DLL 保持不变。

| 项目 | 最终交付记录 |
|---|---|
| 构建源码提交 | `51f1be0ff401d3b9b8e23cb093b8377356abb196` |
| Actions 构建链接 | [成功构建 35087578421](https://github.com/d4go/klogg/actions/runs/35087578421) |
| 程序版本 | 24.11.0.724 |
| 目标架构 / 配置 | Windows x64 / Release |
| Qt | 6.7.3 |
| 编译器 | MSVC 19.44.35228.0，Visual Studio 2022 x64 工具链 |
| 可执行文件 | `klogg.exe`、`klogg_portable.exe`、`klogg_grep.exe`；部署验证通过 |
| 下载链接 | [Windows x64 Qt6 简体中文版](https://github.com/d4go/klogg/releases/tag/zh-cn-24.11.0.724) |
| 测试证据链接 | [验证证据压缩包](https://github.com/d4go/klogg/releases/download/zh-cn-24.11.0.724/klogg-zh_CN-validation.zip) |

## 已知限制

没有为本次交付关闭 Hyperscan，未修改日志读取、索引、搜索、正则或编码检测引擎，也未生成 32 位或其他平台交付包。

- 保留上游编码检测启发式。短 emoji 样本自动检测曾误判为 ISO-8859-7；大于 1 GiB、几乎全 ASCII 且仅末尾含中文的样本曾误判为 Latin1。手动选择 UTF-8 后中文及 emoji 显示正确；本次未修改检测引擎。
- 未在真实英语 Windows 系统上执行首次启动；其他系统 locale 的默认选择由 13 个 locale 输入的 C++ 测试覆盖，中文系统默认语言另有原生运行实测。
- 离屏截图存在 emoji 缺字，但本机原生窗口的系统字体回退已确认可显示中文和彩色 emoji；未给生产 UI 强制指定中文字体。
- 内置上游帮助文档、命令行帮助及 GitHub issue 模板保留英文。
- 更新弹窗的应用提示已翻译；远端返回的更新说明保留原文。
- 程序内“检查更新”仍使用上游 `variar/klogg` 的更新来源；本中文维护版应从 `d4go/klogg` 的 Releases 获取更新。
- 应用错误提示已接入翻译；部分正则表达式引擎、网络或系统返回的第三方诊断可能仍为英文，不改写其技术内容。
- 已有用户命名、日志内容、查询表达式、路径、编码名和技术名称保持原样。
- 繁体翻译沿用上游内容，没有在本任务中全面审校。

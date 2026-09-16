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
| 有效条目 | 395 | 483 |
| 已完成 | 194 | 483 |
| unfinished | 201 | 0 |
| 空翻译 | 3 | 0 |
| source 与 translation 相同 | 13 | 13 |
| obsolete / vanished | 0 | 0 |

保留原文的条目均已人工核对：品牌名、编码名 Unicode、技术名 Qt/Hyperscan、容量单位、分隔符或参数格式。占位符按出现次数核对，包括 `%1`、`%L1`、`%n`；助记键字母（忽略大小写）、换行及 HTML 标签/属性均通过静态检查。

Qt 6.7.3 `lupdate` 已全量扫描 `src`，移除 8 条过期 source，并补齐 96 条新提取文案。`lrelease` 已生成 483 条完成翻译的 `zh_CN.qm`。最终 QM 由 CI 重新生成并嵌入 EXE，不要求用户安装外部语言文件。

## 修复的硬编码英文

恢复默认快捷键确认、快捷键录入、自动编码选项、编码语言分组、快速查找方向及结束提示、搜索结果切换提示、暂存区转换工具/时间/标签命名、新建高亮规则集、崩溃报告提示及超长日志行错误等。

## 语言行为

- 新配置：简体中文 locale → `zh_CN`；繁体中文 locale → 现有 `zh_TW`；其他 → `en`。
- 已有 `view.language`：尊重用户选择，不覆盖。
- 英文：移除翻译器并恢复英文 source，不依赖不存在的 `qt_en.qm`。
- Qt 翻译缺失不会阻止应用翻译加载。
- 设置可切换 English / 中文（简体）；明确提示重启后所有界面完整生效。
- 不替换生产 UI 默认字体，也不打包中文字体。

## 验证状态

- PASS：Qt 6.7.3 `lupdate` 和 `lrelease`。
- PASS：483 条翻译静态检查（0 错误、0 待复核警告）。
- PASS：翻译检查器 7 个正反例回归测试。
- PASS：工作流 YAML / actionlint 和 PowerShell 语法检查。
- PASS：`git diff --check`。
- 已预览：源 `.ui` 设置界面的中文排版（这不等同于完整程序运行验证）。
- 待 CI：MSVC x64 Release 编译、C++ 功能测试、内嵌 QM 与中英文切换、运行包依赖/架构、实际日志读取/搜索及编码测试。

## 构建与产物

工作流：**Build zh_CN Windows x64**，支持 `workflow_dispatch` 和 `zh-cn` 推送。

预期主产物：`klogg-windows-x64-qt6-zh_CN`，包含 EXE、Qt6 DLL、Windows 平台插件和所需运行库；翻译证据产物为 `klogg-windows-x64-qt6-zh_CN-i18n`。

## 已知限制

本报告将在正式 CI 完成后补充真实构建链接和测试结果。当前不把未执行的 Windows 程序手动测试、1GiB 大文件、实时跟踪或编码场景标为 PASS。繁体翻译沿用上游内容，没有在本任务中全面审校。

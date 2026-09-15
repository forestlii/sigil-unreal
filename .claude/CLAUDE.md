# Sigil Unreal 项目工作流

## 项目概览

Sigil Unreal 是通用 UE C++ 源码插件仓；游戏专有规则和集成约束由消费项目维护。
当前工程声明 UE 5.8，插件声明不包含内容资产。

初始化事实依据：commit 845ba842b674c836cab7993f72be74d9bc78189d
的 README.md、Host/Host.uproject、source 下的插件清单和模块文件。
后续任务必须重新核对实际分支、HEAD 和未提交修改，不把初始化快照当作当前状态。

本文件是 Claude/Codex 共用规则入口；具体框架知识由获批的项目 Skill 按需提供。

## 代码结构

- source/：插件源码，公开接口与实现位于各插件的 Source 模块中。
  当前插件为 SigilInput、SigilGas、SigilCombat、SigilMovement、
  SigilInventory、SigilInteraction、SigilCamera、SigilUI、SigilEffects、
  SigilArsenal、SigilSave、SigilNarrative。
  来源：source/*/*.uplugin。
- 模块类型、加载阶段和依赖以对应 .uplugin 与 Source 下的 Build.cs 为准。
  已核对的跨插件依赖：SigilCombat → SigilGas；
  SigilArsenal → SigilGas、SigilInventory、SigilCombat。
  来源：对应插件描述文件及同名模块 Build.cs。
- Host/：验证宿主，包含 Host 与 HostEditor 目标；
  Host.uproject 通过 AdditionalPluginDirectories 的 ../source 引用插件。
  来源：Host/Host.uproject、Host/Source/*.Target.cs。
- docs/：现有中英文使用文档。
- source/<插件>/MD/devlog/decisions.md：已有插件决策记录按任务读取。
  是否版本化以 .gitignore 和 Git 实际状态为准。
- 根目录记忆.md 是已有本地接力文件，受 Git 忽略。
  有文件时先读，但历史状态和验证结论须核对来源；跨设备不能依赖它。
  不另建第二份活记忆，不把记忆当成批准记录。

README 当前未列出 SigilSave、SigilNarrative；入门文档中的九插件和
junction 描述与 Host 清单不同。按实际清单判断结构，不自动修订这些文档。

## 构建与验证

以下是从文件发现的入口，不代表已经执行成功：

- docs/getting-started.md 提供：
  <EngineRoot>/Engine/Build/BatchFiles/Build.bat HostEditor Win64 Development -project="<repo>/Host/Host.uproject" -WaitMutex
- source/SigilArsenal/MD/devlog/decisions.md 记录 Automation 命令片段：
  Automation RunTests Sigil

引擎绝对路径、本机工具链及完整测试启动参数尚未核验。
实际执行前确认路径、目标、验证范围和授权。

2026-09-14 初始化与拆解任务不运行引擎、安装业务依赖、构建、
测试或格式化。后续验证按任务授权执行，并记录命令、commit、
退出状态和证据位置。未运行项写“未验证”，不能用编译结果代替
PIE、联网、Cook、打包运行或消费项目验收。

2026-09-16 在消费项目 ProjectSpecter（gitlink 9ffca22）中，全部 12 个插件随
ProjectSpecterEditor Win64 Development 编译通过并在编辑器加载；
Host 本身未构建，Automation 未运行。

## 开发流程

1. 理解需求：确认职责、来源、验收条件、非目标和涉及的仓库。
2. 只读核对：检查规则入口、Git 状态、实际源码及当前任务匹配的 Skill。
   区分文件证实事实、历史记录和待确认事项。
3. 方案审核：提交范围、接口影响、依赖变化和验证计划。
   方案获批不自动扩大实施、验证或跨仓授权。
4. 实施：只在已明确授权的文件与行为范围内操作，保留现有未提交修改。
   遇到同文件冲突先报告，不切分支、清理或覆盖来消除冲突。
5. 验证与交付：仅运行获准检查，写后回读；报告实际结果及未验证项。
   Git 提交与推送按本仓任务的明确授权处理。

2026-09-14 初始化按 bootstrap-project-workflow 执行：先审核两个完整入口，
获准后写入，再单独审核 Skill 候选，不混写业务 Skill。
来源：Likeon，2026-09-14 的工作台接入任务指令。

## 项目约束

- Sigil 保存通用能力与接口约束；ProjectSpecter 保存游戏专有规则及集成约束。
  跨仓使用明确的来源路径与实际 commit，不复制通用实现说明，
  不因消费项目需求擅自改变仓库边界或接入方式。
- 不顺带重构、不覆盖现有改动；拆文档不构成业务代码或引擎修改授权。
- 保留已有规则、源码、配置和文档；不以模板覆盖。
- 凭据、模型登录信息、.local 和私人知识库内容不进入 Git。
- 个人项目版权署名使用：
  Copyright (c) 2026 Likeon. All Rights Reserved.
- 对外文档沿用英文主文件与 .zh-CN 中文文件，分别完整成篇。
- 提交标题使用：@ type: 中文描述 | English description
- 新文档和工具遵守项目归档规则，不在根目录散放文件。

上述职责、授权、署名及归档约束来源：Likeon 本轮任务及全局项目指令。

工作台相关任务必须显式获得工作台 checkout 路径，并读取其
docs/ONBOARD-LAPTOP.md 与 docs/WORKFLOWS.md；不得假定自动发现兄弟目录。
文档读取不等于云端连接，不复制台式机节点凭据或将 laptop 注册为第二个 home-windows。

## 项目 Skill

现有 13 个项目 Skill（2026-09-15 获批，441ecce 合入 main），覆盖全部 12 个插件，
SigilNarrative 拆为 runtime 与 authoring：
sigil-input、sigil-gas、sigil-combat、sigil-inventory、sigil-arsenal、sigil-movement、
sigil-camera、sigil-effects、sigil-interaction、sigil-ui、sigil-save、
sigil-narrative-runtime、sigil-narrative-authoring。

后续按职责独立、源码路径明确、会重复使用的框架或工作流拆分，
不按目录机械生成，不预设数量。

新增或修改前依次完成：
1. 提交名称、真实触发条件、覆盖与不覆盖范围、来源路径和依赖，供负责人审核。
2. 候选获批后，只读取必要源码，展示完整 SKILL.md 草稿和准确文件列表。
3. 获得明确写入授权后，仅写获批文件。

位置为 .claude/skills/<name>/SKILL.md。
description 必须支持按任务选择；只加载当前任务匹配的 Skill。
辅助文件确有必要再提议。修改共同入口必须单列范围，不随 Skill 写入。

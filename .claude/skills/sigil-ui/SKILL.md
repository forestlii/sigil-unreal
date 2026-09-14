---
name: sigil-ui
description: 在接入、修改或排查 SigilUI 的 GameUIPolicy、玩家布局、LocalPlayer 输入动作、UI Context、模态界面或扩展点时读取。
---

# Sigil UI 基础设施

源码核对基线：845ba842b674c836cab7993f72be74d9bc78189d。
以下是静态源码事实；目标版本不同须重新核对，不代表运行验证通过。

## 来源与读取入口

模块根：source/SigilUI/Source/SigilUI/。

- Public/SigilUISettings.h。
- Public/UI/SigilGameUISubsystem.h 与对应 Private 实现。
- 按任务读取 Public/UI/SigilGameUIPolicy.h、
  SigilGameUILayout.h、SigilGameUIContext.h 及对应实现。
- 扩展任务读取 Public/UIExtension/SigilGameUIExtensionSubsystem.h、
  SigilGameUIExtensionPointWidget.h 及对应实现。

## 已核实的接口约束

- GameUISubsystem.Initialize 要求配置 GameUIPolicyClass；
  未配置时记录错误并返回。
- Policy 类通过 LoadSynchronous 加载，再实例化并切换。
- RegisterUIActionBindingForPlayer 与对应注销方法
  将显式 LocalPlayer 交给 CurrentPolicy。
- RegisterUIContextForPlayer 成功后生成包含 LocalPlayer
  和 ContextClass 的绑定句柄。
- UnregisterUIContextForPlayer 按该句柄移除 Context，
  随后清空句柄中的玩家和类引用。
- 玩家加入、移除和销毁通过对应通知交给 Policy。

## 处理当前任务

先检查 Policy 配置及创建结果，再核对界面所属 LocalPlayer。
动作与 Context 使用其对应句柄和注销路径，不能默认所有 UI 属于首个玩家。
布局、模态层和扩展点问题继续读取实际 Policy、Layout 或扩展实现，
不以通用子系统方法名推定游戏页面行为。

## 范围

覆盖 UI 基础设施。
具体页面、对话显示内容及游戏业务状态由消费项目负责。

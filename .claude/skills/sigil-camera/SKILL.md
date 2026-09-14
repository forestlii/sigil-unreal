---
name: sigil-camera
description: 在接入、修改或排查 SigilCamera 的相机组件、CameraMode 栈、模式切换混合或穿透规避时读取。
---

# Sigil 相机模式

源码核对基线：845ba842b674c836cab7993f72be74d9bc78189d。
以下是静态源码事实；目标版本不同须重新核对，不代表运行验证通过。

## 来源与读取入口

模块根：source/SigilCamera/Source/SigilCamera/。

- Public/SigilCameraSystemComponent.h 与对应 Private 实现。
- Public/SigilCameraModeStack.h 与 Private/SigilCameraModeStack.cpp。
- 穿透任务读取 Public/SigilCameraMode_WithPenetrationAvoidance.h
  及对应实现，不把其他模式的行为类推到该模式。

## 已核实的接口约束

- ModeStack 的 GetCameraModeInstance 按模式类复用实例。
- PushCameraMode 对已经位于栈顶的模式直接返回；
  已在栈内的模式会移动到栈顶，并保留计算出的既有混合贡献。
- PushCameraMode 将栈底权重设为 1；
  新进入栈的模式触发 OnActivation。
- ActivateStack 和 DeactivateStack 将状态变化通知栈内模式。
- EvaluateStack 在栈未激活时返回 false；
  激活时调用 UpdateStack 与 BlendStack。
  不能仅凭返回值推定空栈一定产生有效视图。

## 处理当前任务

从相机组件选择模式的调用点追踪到栈操作、实例状态和最终视图。
模式被复用时检查实例状态是否需要重置；
切换问题同时核对栈次序、权重和激活/停用钩子。
穿透问题单独追踪检测输入、忽略对象及结果处理。

## 范围

覆盖通用相机模式及混合。
游戏镜头设计、对话视角策略和 LevelSequence 演出由消费项目负责。

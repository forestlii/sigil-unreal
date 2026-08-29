# SigilCamera 决策日志

> 本文件只记录 `SigilCamera` 插件内部的审查修复取舍，不记录消费项目玩法、资产或运行时结论。
>
> 最后更新：2026-08-30。

### [2026-08-30] CAM-01 空相机栈不覆盖已有视图

- 阶段: 审查批次 A 修复
- 面临的选择: 激活空栈仍以默认视图继续写入相机组件，或把无有效模式视为没有可应用的视图。
- 定了什么: `EvaluateStack` 在栈为空时返回 `false` 且不写输出 view；`TickComponent` 收到 `false` 后立即返回，因此不会写 Camera、SpringArm 或消费待应用 FOV offset。
- 否掉了什么 + 为什么: 否掉新增默认模式、自动 Push、项目镜头规则或公开 API/反射/复制/序列化改动；这些都超出防止空栈覆盖的最小修复。
- 复用层🔑: ② 引擎相关
- 来源: Finding `CAM-01`；Sigil commit `c385770`；`SigilCamera.Stack` Automation（3/3 通过，见审查批次 A Task 4 报告）。

# SigilEffects 决策日志

> 本文件只记录 `SigilEffects` 插件内部的设计取舍，不记录消费项目的玩法、资产或产品范围。
>
> 记账规则来自第二大脑 `workflow/tools/游戏开发决策记录-模板.md`。最后更新：2026-09-11。
>
> 本轮（2026-09-11）条目来源：外部审计 `D:\P4\Code_UE5.6\MD\analysis\gasshooter-sigil-borrow-audit.md`（EXT-20260911-GASSHOOTER-BORROW-AUDIT-001，R2）。

### [2026-09-11] B4：情景效果通知按视角过滤，视角接口放在 SigilEffects 自己而不是 SigilGas

- 阶段: 迭代
- 面临的选择: 审计建议"视角查询做成 SigilGas 接口"；但 SigilEffects 与 SigilGas 互不依赖（`docs/getting-started.md` 依赖图：只有 combat→gas，其余全独立）。选择：① 让 SigilEffects 依赖 SigilGas；② 接口放 SigilEffects；③ 抄 GASShooter 直接 `Cast<AGSHeroCharacter>`。
- 定了什么: ②——新增 `ISigilViewPerspectiveInterface::IsInFirstPersonPerspective()`（Blueprintable，可实现在 Pawn 或其组件上）；`USigilAnimNotify_ContextEffects` 新增 `PerspectiveFilter`（`Any` 默认 / `FirstPersonOnly` / `ThirdPersonOnly`）。判定规则（改编自 GASShooter `GSAnimNotify_PlaySoundForPerspective`，Copyright 2020 Dan Kestranek，MIT）：`bLocalFirstPerson = 本地控制 && 找到实现者 && 汇报第一人称`；`FirstPersonOnly` 只在 `bLocalFirstPerson` 时播，`ThirdPersonOnly` 取反。解析沿拥有链走（Actor → 其组件 → `GetOwner()`，最多 8 层，兜底 `GetInstigator()`），武器 / 装备 Actor 上的 Mesh 通知也能找到 Pawn 的视角。非游戏世界（动画编辑器预览）总是播放。
- 否掉了什么 + 为什么: 否掉①——为一个布尔查询引入 GAS 依赖，破坏"其余包全独立"的承诺；否掉③——把插件耦合到具体角色类，违背组件 + 接口范式；否掉 GASShooter 里"远端玩家 Pawn 也可能播 FP 音效"的分支——语义含混，简化为"只有本地第一人称观看者算 FP"。无实现者时按世界身体处理（`ThirdPersonOnly` 播、`FirstPersonOnly` 不播），误配时保守。
- 复用层🔑: ② 引擎相关
- 来源: 审计 §2 B4；GASShooter `GSAnimNotify_PlaySoundForPerspective.cpp:16-70`；Automation `SigilEffects.ContextEffects.PerspectiveFilterTable`、`SigilEffects.ContextEffects.PerspectiveResolvesThroughOwnerChain`。

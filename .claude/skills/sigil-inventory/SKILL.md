---
name: sigil-inventory
description: 在接入、修改或排查 SigilInventory 的物品定义与 Fragment、实例和容器、装备拾取、交易合成、复制或物品序列化时读取。
---

# Sigil 物品与容器

源码核对基线：845ba842b674c836cab7993f72be74d9bc78189d。
以下是静态源码事实；目标版本不同须重新核对，不代表运行验证通过。

## 来源与读取入口

模块根：source/SigilInventory/Source/SigilInventory/。

- Public/SigilInventoryFactory.h 与 Private/SigilInventoryFactory.cpp。
- Public/SigilInventorySystemComponent.h 与对应 Private 实现。
- 物品、容器读取 Public/Core/Items、Public/Core/Collections 及对应实现。
- 装备、合成、交易分别按任务读取 Equipping、Crafting、Exchange。
- 拾取实现：Private/Pickups/SigilItemPickupComponent.cpp。
- 序列化声明：Public/Serialization/SigilSerializationStructLibrary.h。
- 依赖来源：SigilInventory.Build.cs；本模块不依赖 SigilGas。

## 已核实的接口约束

- Factory.CreateItem 检查 Owner 和 Definition，创建实例、
  分配 GUID、设置定义，初始化 Fragment 的 Mixin 状态，
  然后调用 Fragment.OnInstanceCreated。
  仅调用 SetDefinition 不能代替这条完整创建路径。
- ItemPickupComponent.Pickup 检查 Owner authority。
- 拾取路径检查 CanAddItem 和是否要求整份容纳；
  实际 AddItem 返回正数量后才通知拾取成功。
- InventorySystemComponent 声明了子对象复制入口；
  具体对象归属和注册顺序须继续核对其实现。

## 处理当前任务

区分 Definition、运行时 Item、Collection 和 Equipment 的职责。
数量修改记录请求量、可接受量和实际结果；同时检查失败路径。
复制、序列化、交易或合成只读取对应链路，不据方法名称推定一致性保证。
装备授予能力属于 sigil-arsenal，不把 GAS 依赖下沉到基础物品层。

## 范围

覆盖物品与容器机制。
不负责整局存档、自动保存时机或游戏专有经济与装备规则。

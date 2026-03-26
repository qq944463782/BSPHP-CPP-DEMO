# BSPHP-Cpp-System-Card-DEMO

![目录结构图](../docs/project-structure.svg)

卡模式系统层调用演示，包含：

- 完整 BSPHP 请求链路（加密/会话）
- 驱动层 IOCTL 调用命令（`ioctl_install` / `ioctl_verify` / `ioctl_recharge`）

## 配置初始化文件

- 地址与密钥初始化文件：`core/card_config.cpp`
- 主要配置项：
  - `kUrl`
  - `kMutual`
  - `kServerKey` / `kClientKey`

## 补充说明

- 驱动调用说明见：`驱动调用使用说明.md`（仓库根目录）


# BSPHP-Cpp-CMD-Card-DEMO

![目录结构图](../docs/project-structure.svg)

卡模式命令行（CMD）演示，包含 `login.ic`、`getdate.ic`、心跳、绑定/解绑、购卡链接等。

## 配置初始化文件

- 地址与密钥初始化文件：`core/card_config.cpp`
- 主要配置项：
  - `kUrl`（接口地址）
  - `kMutual`（通信互验 Key）
  - `kServerKey` / `kClientKey`（RSA 密钥）

## 运行

- 打开 `BSPHP-Cpp-CMD-Card-DEMO.sln`
- 选择 `Debug|x64` 编译运行


# BSPHP-Cpp-CMD-Login-DEMO

![目录结构图](../docs/project-structure.svg)

登录模式命令行（CMD）演示，包含 `login.lg`、`getuserinfo.lg`、`timeout.lg`、验证码开关等。

## 配置初始化文件

- 地址与密钥初始化文件：`core/login_demo_config.cpp`
- 主要配置项：
  - `kBSPHPURL`（接口地址）
  - `kMutualKey`（通信互验 Key）
  - `kServerKey` / `kClientKey`（RSA 密钥）
  - `kCodeURLPrefix`（验证码地址前缀）

## 运行

- 打开 `BSPHP-Cpp-CMD-Login-DEMO.sln`
- 选择 `Debug|x64` 编译运行


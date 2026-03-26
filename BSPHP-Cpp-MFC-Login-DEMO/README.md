# BSPHP-Cpp-MFC-Login-DEMO

![目录结构图](../docs/project-structure.svg)

登录模式 MFC GUI 演示，包含登录、注册、短信/邮箱相关流程与控制台功能。

## 配置初始化文件

- 地址与密钥初始化文件：`core/login_demo_config.cpp`
- 主要配置项：
  - `kBSPHPURL`
  - `kMutualKey`
  - `kServerKey` / `kClientKey`
  - `kCodeURLPrefix`

## 入口

- 主工程：`app/LoginMfcApp.vcxproj`
- 界面逻辑：`app/*.cpp`


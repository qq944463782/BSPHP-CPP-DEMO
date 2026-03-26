# BSPHP · Apple iOS / macOS Demo

[www.bsphp.com](https://www.bsphp.com)

[BSPHP](https://www.bsphp.com) 为软件会员/订阅与授权管理方案，支持账号密码注册、充值卡激活等多种方式。

---

## 目录 · Table of contents

README.md 写项目说明 支持 简体中文 | [简体中文](#简体中文)

| Language | Section |
|----------|---------|
| 简体中文 | [简体中文](#简体中文) |
| 繁體中文 | [繁體中文](#繁體中文) |
| English | [English](#english) |

---

## 简体中文

本仓库是 BSPHP C++ 多端演示集合，包含 GUI、命令行和系统层调用示例。  
每个目录都是独立工程，可单独打开 `.sln` 进行编译和运行。

### 项目列表

- `BSPHP-Cpp-MFC-Card-DEMO`：卡模式 MFC GUI 演示
- `BSPHP-Cpp-MFC-Login-DEMO`：登录模式 MFC GUI 演示
- `BSPHP-Cpp-win-Card-DEMO`：卡模式 Win32 GUI 演示
- `BSPHP-Cpp-win-Login-DEMO`：登录模式 Win32 GUI 演示
- `BSPHP-Cpp-CMD-Card-DEMO`：卡模式 CMD 演示
- `BSPHP-Cpp-CMD-Login-DEMO`：登录模式 CMD 演示
- `BSPHP-Cpp-System-Card-DEMO`：卡模式系统/驱动层调用演示（含 IOCTL 命令）
- `BSPHP-Cpp-System-Login-DEMO`：登录模式系统/驱动层调用演示（含 IOCTL 命令）

### 使用说明

- 推荐环境：Visual Studio 2019/2022，`x64`
- 打开对应项目目录下 `.sln` 后直接编译运行
- `System` 项目同时支持：
  - 完整 BSPHP 请求链路（配置/会话/加密流程）
  - 驱动通信示例（`DeviceIoControl`）

### 备注

如果需要 DLL 验证，只需向 DLL 传递 `bsSeSsl` 即可实现 DLL 验证，随后通过 DLL 调用接口即可。

---

## 繁體中文

本倉庫為 BSPHP C++ 多端示範集合，包含 GUI、命令列與系統層呼叫範例。  
每個目錄都是獨立工程，可單獨開啟 `.sln` 進行編譯與執行。

### 專案列表

- `BSPHP-Cpp-MFC-Card-DEMO`：卡模式 MFC GUI
- `BSPHP-Cpp-MFC-Login-DEMO`：登入模式 MFC GUI
- `BSPHP-Cpp-win-Card-DEMO`：卡模式 Win32 GUI
- `BSPHP-Cpp-win-Login-DEMO`：登入模式 Win32 GUI
- `BSPHP-Cpp-CMD-Card-DEMO`：卡模式 CMD
- `BSPHP-Cpp-CMD-Login-DEMO`：登入模式 CMD
- `BSPHP-Cpp-System-Card-DEMO`：卡模式系統/驅動層呼叫（含 IOCTL）
- `BSPHP-Cpp-System-Login-DEMO`：登入模式系統/驅動層呼叫（含 IOCTL）

---

## English

This repository is a BSPHP C++ demo collection across multiple runtime styles:

- MFC GUI demos
- Win32 GUI demos
- Console (CMD) demos
- System/driver-layer demos (with IOCTL examples)

Each project is independent and can be built/run from its own `.sln`.

### Note

For DLL-based verification, pass `bsSeSsl` into your DLL and perform verification through DLL APIs.


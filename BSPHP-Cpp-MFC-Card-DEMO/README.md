# BSPHP-Cpp-MFC-Card-DEMO 项目说明

## 简体中文
该项目是一个基于 **MFC** 的“卡片(Card)”登录/交互演示 Demo。UI 层位于 `app/`，核心 BSPHP API 客户端、HTTP 加密通信等实现位于 `core/`。程序构建完成后，在 `bin/` 目录下生成可执行文件。

### 目录结构
```text
BSPHP-Cpp-MFC-Card-DEMO/             （项目根目录）
├─ app/                              （MFC UI 与入口工程）
│  ├─ *.cpp / *.h                   （对话框、应用类、业务页面等）
│  ├─ *.rc / resource.h            （资源、控件与菜单等）
│  └─ CardMfcApp.vcxproj            （VS 工程文件）
├─ core/                             （BSPHP 客户端与加密/HTTP 通信）
│  ├─ bsphp_client.*               （BSphp API 客户端：connect/login/register/api_call）
│  ├─ crypto_http.cpp              （MD5/AES/RSA/Base64 等 + WinHTTP POST 实现）
│  ├─ card_config.*               （卡片配置/URL 生成相关）
│  └─ machine_id.*                （Windows 机器标识，用于登录.ic 的 key/maxoror）
├─ bin/                              （编译产物输出目录）
│  └─ Debug/                         （当前目录下已有示例产物）
├─ obj/                              （中间目标文件目录）
├─ enc_temp_folder/                 （加密/临时内容目录，演示用）
└─ BSPHP-Cpp-MFC-Card-DEMO.sln      （Visual Studio 解决方案）
```

### 主要源码文件（按目录）
`app/`
- `CardMfcApp.cpp`：MFC 应用入口相关
- `MainCardDlg.*`：主卡片界面对话框
- `PanelDlg.*`：顶部/面板式控制对话框
- `MachineTabDlg.*`：机器/设备相关 Tab 页面
- `CardTabDlg.*`：卡片 Tab 页面

`core/`
- `bsphp_client.h/cpp`：BSPHP API 客户端类 `bsphp::BsPhp`
- `card_config.h/cpp`：演示密钥/卡片相关配置与 URL 生成
- `crypto_http.cpp`：加密 + WinHTTP POST 通信实现
- `machine_id.h/cpp`：生成用于登录校验的机器标识

### 结果/产物文件说明
当前构建产物位于：
- `bin\Debug\CardMfcApp.exe`：演示程序可执行文件
- `bin\Debug\CardMfcApp.pdb`：调试符号文件（用于 Debug）

说明：
- `obj/` 目录为中间编译产物（不建议提交/无需手动使用）。
- `enc_temp_folder/` 用于存放演示过程中的临时/加密相关内容（目录下当前包含一个子目录以及 `card_config.cpp` 示例）。

### 构建与运行
1. 使用 Visual Studio 打开 `BSPHP-Cpp-MFC-Card-DEMO.sln`
2. 选择 `Debug` 或 `Release`，执行“生成/Build”
3. 运行 `bin\<Configuration>\CardMfcApp.exe`

---

## 繁體中文
此專案是基於 **MFC** 的「卡片(Card)」登入/互動示範 Demo。UI 介面位於 `app/`，核心 BSPHP API 客戶端、HTTP 加密通訊等實作位於 `core/`。程式建置完成後會在 `bin/` 目錄產生可執行檔。

### 目錄結構
```text
BSPHP-Cpp-MFC-Card-DEMO/             （專案根目錄）
├─ app/                              （MFC UI 與入口工程）
│  ├─ *.cpp / *.h                   （對話框、應用類、業務頁面等）
│  ├─ *.rc / resource.h            （資源、控制項與選單等）
│  └─ CardMfcApp.vcxproj            （VS 專案檔）
├─ core/                             （BSPHP Client 與加密/HTTP 通訊）
│  ├─ bsphp_client.*               （BSphp API Client：connect/login/register/api_call）
│  ├─ crypto_http.cpp              （MD5/AES/RSA/Base64 等 + WinHTTP POST 實作）
│  ├─ card_config.*               （卡片設定/URL 產生相關）
│  └─ machine_id.*                （Windows 裝置識別，用於 login.ic 的 key/maxoror）
├─ bin/                              （編譯產物輸出目錄）
│  └─ Debug/                         （目前已存在示例產物）
├─ obj/                              （中間目標檔目錄）
├─ enc_temp_folder/                 （加密/暫存內容目錄，示範用）
└─ BSPHP-Cpp-MFC-Card-DEMO.sln      （Visual Studio 解決方案）
```

### 主要原始碼檔案（依目錄）
`app/`
- `CardMfcApp.cpp`：MFC 應用程式入口相關
- `MainCardDlg.*`：主要卡片介面對話框
- `PanelDlg.*`：上方/面板式控制對話框
- `MachineTabDlg.*`：機器/裝置相關 Tab 頁面
- `CardTabDlg.*`：卡片 Tab 頁面

`core/`
- `bsphp_client.h/cpp`：BSPHP API 客戶端類別 `bsphp::BsPhp`
- `card_config.h/cpp`：示範金鑰/卡片相關設定與 URL 生成
- `crypto_http.cpp`：加密 + WinHTTP POST 通訊實作
- `machine_id.h/cpp`：產生用於登入驗證的裝置識別

### 結果/產物檔案說明
目前建置產物位於：
- `bin\Debug\CardMfcApp.exe`：示範程式可執行檔
- `bin\Debug\CardMfcApp.pdb`：除錯符號檔（用於 Debug）

說明：
- `obj/` 目錄為中間編譯產物（不建議提交/也不需要手動使用）。
- `enc_temp_folder/` 用於存放示範流程中的暫存/加密相關內容（目前包含一個子目錄與 `card_config.cpp` 範例）。

### 建置與執行
1. 使用 Visual Studio 開啟 `BSPHP-Cpp-MFC-Card-DEMO.sln`
2. 選擇 `Debug` 或 `Release`，執行「建置/Build」
3. 執行 `bin\<Configuration>\CardMfcApp.exe`

---

## English
This project is a **MFC**-based “Card” login/interaction demo. The UI layer is located in `app/`, while the core BSPHP API client and encrypted HTTP communication are implemented in `core/`. After building, the executable is output to `bin/`.

### Directory Structure
```text
BSPHP-Cpp-MFC-Card-DEMO/             (Project root)
├─ app/                              (MFC UI and VS entry project)
├─ core/                             (BSPHP client + crypto/HTTP)
├─ bin/                              (Build outputs)
├─ obj/                              (Intermediate build artifacts)
├─ enc_temp_folder/                 (Encryption/temporary demo data)
└─ BSPHP-Cpp-MFC-Card-DEMO.sln      (Visual Studio solution)
```

### Key Source Files
`app/`
- `CardMfcApp.cpp`: MFC application entry
- `MainCardDlg.*`: main card dialog
- `PanelDlg.*`: panel/control dialog
- `MachineTabDlg.*`: machine/device tab
- `CardTabDlg.*`: card tab

`core/`
- `bsphp_client.*`: `bsphp::BsPhp` API client (connect/login/register/api_call)
- `crypto_http.cpp`: crypto (MD5/AES/RSA/Base64) + WinHTTP POST implementation
- `card_config.*`: card configuration / demo URL generation
- `machine_id.*`: Windows machine id used for `login.ic` key/maxoror

### Build Outputs (Result Files)
Current outputs are in:
- `bin\Debug\CardMfcApp.exe`: demo executable
- `bin\Debug\CardMfcApp.pdb`: debug symbols

Notes:
- `obj/` contains intermediate compilation artifacts.
- `enc_temp_folder/` contains temporary/encryption-related demo contents.

### Build & Run
1. Open `BSPHP-Cpp-MFC-Card-DEMO.sln` in Visual Studio
2. Build for `Debug` or `Release`
3. Run `bin\<Configuration>\CardMfcApp.exe`


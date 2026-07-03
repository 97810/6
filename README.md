# F2 一键断网：VS Code 可修改学习版

这个项目不是原 exe 的作者源码，而是根据静态反编译结果整理出的 **可编译 Win32/C++ 学习版**。它复现的是目前能确认的核心逻辑：

1. 程序注册全局热键 `F2`；
2. 第一次按 `F2` 时，读取当前鼠标所在窗口句柄与窗口内坐标；
3. 后续按点击键时，默认向绑定窗口发送后台鼠标消息，不移动真实鼠标；
4. 提供“重置”和“退出”按钮，并添加托盘图标。

> 注意：它并不是直接调用 `netsh`、IP Helper、WLAN API 去禁用网卡。根据反编译字符串和导入函数，原程序更像是“绑定某个网络开关位置，然后用 F2 自动点击”。

## 文件结构

```text
F2_NetworkSwitch_Rebuild/
├── CMakeLists.txt
├── README.md
├── CODEX_PROMPT.md
├── src/
│   ├── main.cpp
│   ├── F2ClickerApp.h
│   └── F2ClickerApp.cpp
├── resources/
│   ├── app.rc
│   └── app.manifest
├── docs/
│   ├── reverse_engineering_notes.md
│   └── pseudocode_from_decompile.cpp
└── .vscode/
    ├── tasks.json
    └── launch.json
```

## 在 VS Code 中打开

1. 解压本项目文件夹。
2. 用 VS Code 打开 `F2_NetworkSwitch_Rebuild` 文件夹。
3. 推荐安装：
   - C/C++ Extension Pack
   - CMake Tools
4. Windows 上准备编译环境：
   - Visual Studio 2022 Build Tools，选择 “Desktop development with C++”；或
   - MinGW-w64。
5. 在 VS Code 终端运行：

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

生成文件一般在：

```text
build/Release/F2NetworkSwitch.exe
```

## 使用方式

1. 运行程序。
2. 把鼠标放到你要绑定的开关位置。
3. 按 `F2`，程序会记录窗口句柄和窗口内坐标。
4. 后续按点击键，程序会对记录的位置发送一次后台点击并切换计时状态。
5. 后台点击模式固定启用，不会切换到真实鼠标输入。
6. 点击“重置”可以重新绑定。

## 后台点击兼容性

后台模式使用 `PostMessageW` 向绑定的窗口或控件发送 `WM_LBUTTONDOWN` 和 `WM_LBUTTONUP`，不会调用 `SetCursorPos`，也不会把点击注入当前前台输入流。

并非所有程序都会处理这种窗口消息。DirectX/Raw Input 游戏、部分浏览器复杂页面、自绘界面以及权限级别更高的程序可能忽略后台点击。Windows 只能确认消息是否成功加入目标线程的消息队列，无法确认目标业务逻辑是否实际响应。如果目标程序以管理员身份运行，本程序也需要以管理员身份运行。

## 关于管理员权限

`resources/app.manifest` 默认使用：

```xml
<requestedExecutionLevel level="requireAdministrator" uiAccess="false"/>
```

这是因为原程序 manifest 疑似需要管理员权限。如果你只是学习代码，不想每次弹出 UAC，可以改成：

```xml
<requestedExecutionLevel level="asInvoker" uiAccess="false"/>
```

## 后续适合让 Codex 修改的方向

- 把热键从固定 `F2` 改成可配置；
- 增加保存/读取坐标配置；
- 增加多目标绑定；
- 改成真正调用 Windows 网络 API 禁用/启用指定网卡；
- 增加日志输出，记录每次绑定和点击；
- 优化界面布局和中文显示。

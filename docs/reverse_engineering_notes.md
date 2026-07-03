# F2一键断网.exe 反编译/静态分析说明

## 结论先说

这个文件不能直接还原出作者的完整原始源码。它是一个 **Windows x64 GUI 原生程序**，并且经过 **UPX 3.96 加壳/压缩**。我已经做了静态解壳和反汇编分析，并整理了一个“伪源码版”的 C++ 逻辑文件，方便学习程序结构。

本次给出的 `F2一键断网_伪源码.cpp` 不是原作者源码，而是根据解壳后的字符串、导入函数、资源、关键反汇编逻辑整理出来的近似源码。

## 文件基本信息

- 文件名：F2一键断网.exe
- 类型：PE32+ Windows x64 GUI 程序
- SHA-256：338f182afa4d7fd35489be27babc809a18128a4b28a8a870fbdd8365be8e132d
- 加壳信息：UPX 3.96
- 入口点：原始入口位于 UPX 壳代码；解壳后转移到约 `0x140030ad8`
- Manifest：`requireAdministrator`，即程序请求管理员权限

## 资源信息

资源表中主要是：

- 图标资源
- 版本信息
- Manifest 清单

没有发现可直接抽出的 Python、AutoHotkey、.NET IL、脚本源码或 RCDATA 形式的源码资源。

## 框架/编译痕迹

解壳后的字符串中可见以下框架/类名痕迹：

- `ATL`
- `WTL`
- `CVolUserApp`
- `CVolObject`
- `CVolWinForm`
- `CWtlWndObject`
- `CVolBaseDataType`
- `CVolException`

因此它更像是用 C++/WTL 或类似“Vol/CVol”框架生成的 Windows 原生 GUI 程序，而不是 .NET、Python 或普通脚本打包程序。

## 关键字符串

解壳后可见的核心 UI 字符串包括：

```text
我的主窗口
热    键：F2
重置
热键开关
将鼠标放在开关处按F2
    坐标：
句柄：
信息:
    句柄绑定成功！
菜单
退出
```

这些字符串说明程序的核心功能大概率是：

1. 显示一个“热键开关”窗口；
2. 提示用户“将鼠标放在开关处按F2”；
3. 第一次按 F2 时，记录鼠标所在窗口的句柄与坐标；
4. 后续按 F2 时，对已绑定的窗口/坐标执行模拟点击；
5. 提供“重置”和托盘菜单“退出”。

## 关键 API

解壳后能看到的关键 API 包括：

```text
SetWindowsHookExW
CallNextHookEx
UnhookWindowsHookEx
GetAsyncKeyState
SendInput
ScreenToClient
ClientToScreen
GetWindowThreadProcessId
Shell_NotifyIconW
CreateWindowExW
MessageBoxW
```

这些 API 指向的逻辑是：

- 使用全局键盘钩子监听热键；
- 用 F2 作为热键；
- 获取鼠标位置对应的窗口句柄和客户区坐标；
- 通过 `SendInput` 模拟鼠标/键盘输入；
- 使用系统托盘图标和菜单。

## 是否直接禁用网卡？

从静态分析结果看，没有明显发现以下直接断网相关调用或字符串：

```text
netsh
CreateProcessW / ShellExecuteW 启动 netsh
IP Helper API
WLAN API
Netapi32
```

所以这个程序的“一键断网”更可能不是直接调用系统网络接口禁用网卡，而是通过“绑定某个界面开关，然后模拟点击”的方式实现。例如用户把鼠标放到 Windows 网络开关、某个第三方网络开关或快捷开关上，程序记录该位置，之后按 F2 自动点击。

## 给出的文件说明

- `F2一键断网_伪源码.cpp`：根据反汇编和字符串整理的 C++ 伪源码，适合学习程序逻辑。
- `F2_unpacked_dump_fast.exe`：解壳后的内存转储 PE，用于 Ghidra/IDA/x64dbg 学习分析。注意：这是分析用文件，导入表没有完整重建，不保证能直接运行。
- `F2_disasm.txt`：解壳后转储文件的反汇编文本，体积较大，适合搜索 API 调用位置。
- `import_map.json`：解壳仿真过程中解析到的导入函数映射。

## 学习建议

建议用 Ghidra 打开 `F2_unpacked_dump_fast.exe`，然后重点搜索：

```text
SetWindowsHookExW
SendInput
Shell_NotifyIconW
ScreenToClient
ClientToScreen
```

重点看调用 `SendInput` 的函数，它基本对应“模拟点击/切换开关”的核心逻辑；再看 `SetWindowsHookExW` 附近的回调函数，可以理解 F2 热键是如何触发的。

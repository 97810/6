/*
  F2一键断网.exe —— 伪源码/逻辑还原版

  说明：
  1. 这不是作者的原始工程源码，也不是可以直接编译运行的完整项目。
  2. 这是根据解壳后的 PE、字符串、导入函数和关键反汇编逻辑整理出的“接近源码的伪代码”。
  3. 原程序是 Windows x64 GUI 程序，使用 UPX 3.96 加壳；解壳后可见 ATL/WTL/CVol 等框架痕迹。
  4. 从可见 API 与字符串看，它不像是直接调用 netsh / IP Helper / WLAN API 去禁用网卡，
     更像是：用户把鼠标放在某个“网络开关”上，按 F2 绑定窗口句柄与坐标；之后按 F2 通过 SendInput 模拟点击该开关。
*/

#include <windows.h>
#include <shellapi.h>

class CF2NetworkSwitchApp {
private:
    HWND  mainWindow       = nullptr;
    HWND  boundWindow      = nullptr;
    POINT boundClientPoint = {0, 0};
    HHOOK keyboardHook     = nullptr;
    bool  hasBoundTarget   = false;
    bool  hotkeyEnabled    = true;

public:
    int Run(HINSTANCE hInstance) {
        // Manifest 中声明 requireAdministrator，原程序启动时会请求管理员权限。
        InitCommonControlsAndFramework();
        CreateMainWindow(hInstance);
        CreateTrayIcon();

        SetLabelText(L"热    键：F2\r\n");
        SetHintText(L"将鼠标放在开关处按F2");

        keyboardHook = SetWindowsHookExW(
            WH_KEYBOARD_LL,
            LowLevelKeyboardProc,
            hInstance,
            0
        );

        MSG msg{};
        while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        Cleanup();
        return static_cast<int>(msg.wParam);
    }

private:
    void InitCommonControlsAndFramework() {
        // 解壳后可见 ATL / WTL / CVol 相关类名，例如：
        // CVolUserApp, CVolObject, CVolWinForm, CWtlWndObject 等。
    }

    void CreateMainWindow(HINSTANCE hInstance) {
        // 原程序创建一个 GUI 主窗口。
        // 解壳后可见窗口/控件文本：
        //   “我的主窗口”
        //   “热键开关”
        //   “重置”
        //   “热    键：F2”
        //   “将鼠标放在开关处按F2”
        //   “坐标：” “句柄：” “信息:”
        mainWindow = CreateWindowExW(
            0,
            L"MainWindowClass",
            L"热键开关",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            420, 240,
            nullptr,
            nullptr,
            hInstance,
            nullptr
        );
    }

    void CreateTrayIcon() {
        // 原程序导入并调用 Shell_NotifyIconW，说明有系统托盘图标。
        // 解壳字符串中菜单包含：
        //   菜单
        //   退出
        NOTIFYICONDATAW nid{};
        nid.cbSize = sizeof(nid);
        nid.hWnd = mainWindow;
        nid.uID = 1;
        nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
        nid.uCallbackMessage = WM_USER + 1;
        lstrcpyW(nid.szTip, L"热键开关");
        Shell_NotifyIconW(NIM_ADD, &nid);
    }

    void BindTargetUnderMouse() {
        POINT screenPoint{};
        GetCursorPos(&screenPoint);

        HWND target = WindowFromPoint(screenPoint);
        if (!target) {
            SetInfoText(L"信息: 未找到鼠标所在窗口");
            return;
        }

        POINT clientPoint = screenPoint;
        ScreenToClient(target, &clientPoint);

        boundWindow = target;
        boundClientPoint = clientPoint;
        hasBoundTarget = true;

        wchar_t buffer[256]{};
        wsprintfW(buffer, L"    坐标：%d, %d", clientPoint.x, clientPoint.y);
        SetCoordinateText(buffer);

        wsprintfW(buffer, L"句柄：0x%p", target);
        SetHandleText(buffer);

        SetInfoText(L"信息:    句柄绑定成功！");
    }

    void ToggleBoundTarget() {
        if (!hasBoundTarget || !boundWindow || !IsWindow(boundWindow)) {
            SetInfoText(L"信息: 请先将鼠标放在开关处按F2绑定");
            return;
        }

        POINT screenPoint = boundClientPoint;
        ClientToScreen(boundWindow, &screenPoint);

        // 反汇编中可见 SendInput 调用。这里用鼠标移动 + 左键点击模拟原程序核心逻辑。
        INPUT inputs[3]{};

        inputs[0].type = INPUT_MOUSE;
        inputs[0].mi.dx = NormalizeAbsoluteX(screenPoint.x);
        inputs[0].mi.dy = NormalizeAbsoluteY(screenPoint.y);
        inputs[0].mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;

        inputs[1].type = INPUT_MOUSE;
        inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

        inputs[2].type = INPUT_MOUSE;
        inputs[2].mi.dwFlags = MOUSEEVENTF_LEFTUP;

        SendInput(3, inputs, sizeof(INPUT));
    }

    static LRESULT CALLBACK LowLevelKeyboardProc(int code, WPARAM wParam, LPARAM lParam) {
        if (code >= 0 && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
            KBDLLHOOKSTRUCT* kb = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

            // UI 字符串显示热键为 F2。
            // 程序也导入 GetAsyncKeyState / SetWindowsHookExW / CallNextHookEx。
            if (kb && kb->vkCode == VK_F2) {
                CF2NetworkSwitchApp* app = GetGlobalAppInstance();
                if (app && app->hotkeyEnabled) {
                    if (!app->hasBoundTarget) {
                        app->BindTargetUnderMouse();
                    } else {
                        app->ToggleBoundTarget();
                    }
                    return 1; // 是否拦截按键取决于原程序细节；这里按典型热键工具处理。
                }
            }
        }
        return CallNextHookEx(nullptr, code, wParam, lParam);
    }

    void ResetBinding() {
        boundWindow = nullptr;
        boundClientPoint = {0, 0};
        hasBoundTarget = false;
        SetCoordinateText(L"    坐标：");
        SetHandleText(L"句柄：");
        SetInfoText(L"信息:");
    }

    void Cleanup() {
        if (keyboardHook) {
            UnhookWindowsHookEx(keyboardHook);
            keyboardHook = nullptr;
        }

        NOTIFYICONDATAW nid{};
        nid.cbSize = sizeof(nid);
        nid.hWnd = mainWindow;
        nid.uID = 1;
        Shell_NotifyIconW(NIM_DELETE, &nid);
    }

    int NormalizeAbsoluteX(int x) {
        return x * 65535 / max(1, GetSystemMetrics(SM_CXSCREEN) - 1);
    }

    int NormalizeAbsoluteY(int y) {
        return y * 65535 / max(1, GetSystemMetrics(SM_CYSCREEN) - 1);
    }

    void SetLabelText(const wchar_t* text)      { /* 更新界面文本 */ }
    void SetHintText(const wchar_t* text)       { /* 更新提示文本 */ }
    void SetInfoText(const wchar_t* text)       { /* 更新信息文本 */ }
    void SetCoordinateText(const wchar_t* text) { /* 更新坐标文本 */ }
    void SetHandleText(const wchar_t* text)     { /* 更新句柄文本 */ }

    static CF2NetworkSwitchApp* GetGlobalAppInstance() {
        // 原程序可能由框架保存全局 App / Window 指针。
        return nullptr;
    }
};

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
    CF2NetworkSwitchApp app;
    return app.Run(hInstance);
}

#pragma once

#include <windows.h>
#include <string>

class HotkeyManager {
public:
    static constexpr int kClickHotkeyId = 1001;
    static constexpr int kQuickHotkeyId = 1002;

    explicit HotkeyManager(HWND window = nullptr);
    ~HotkeyManager();

    void SetWindow(HWND window);
    bool Register(UINT click_key, UINT quick_key, std::wstring& error);
    void Unregister();

private:
    static LRESULT CALLBACK MouseHookProc(int code, WPARAM wparam, LPARAM lparam);
    static LRESULT CALLBACK KeyboardHookProc(int code, WPARAM wparam, LPARAM lparam);
    bool RegisterCurrent(UINT click_key, UINT quick_key);
    bool Restore(UINT click_key, UINT quick_key);
    static bool IsMouseKey(UINT key);

    HWND window_ = nullptr;
    UINT click_key_ = 0;
    UINT quick_key_ = 0;
    bool registered_ = false;
    HHOOK mouse_hook_ = nullptr;
    HHOOK keyboard_hook_ = nullptr;
    bool quick_key_down_ = false;

    static HotkeyManager* active_mouse_manager_;
    static HotkeyManager* active_keyboard_manager_;
};

#include "HotkeyManager.h"

HotkeyManager* HotkeyManager::active_mouse_manager_ = nullptr;

HotkeyManager::HotkeyManager(HWND window) : window_(window) {}

HotkeyManager::~HotkeyManager() {
    Unregister();
}

void HotkeyManager::SetWindow(HWND window) {
    window_ = window;
}

bool HotkeyManager::Register(UINT click_key, UINT quick_key, std::wstring& error) {
    if (!window_ || click_key == 0 || quick_key == 0) {
        error = L"热键参数无效。";
        return false;
    }
    if (click_key == quick_key) {
        error = L"点击键和快速键不能相同。";
        return false;
    }

    const UINT old_click = click_key_;
    const UINT old_quick = quick_key_;
    const bool had_old = registered_;
    Unregister();

    if (!RegisterCurrent(click_key, quick_key)) {
        error = L"热键注册失败，可能已被其他程序占用。";
        if (had_old) Restore(old_click, old_quick);
        return false;
    }
    return true;
}

void HotkeyManager::Unregister() {
    if (registered_ && window_) {
        if (!IsMouseKey(click_key_)) UnregisterHotKey(window_, kClickHotkeyId);
        if (!IsMouseKey(quick_key_)) UnregisterHotKey(window_, kQuickHotkeyId);
    }
    if (mouse_hook_) {
        UnhookWindowsHookEx(mouse_hook_);
        mouse_hook_ = nullptr;
        if (active_mouse_manager_ == this) active_mouse_manager_ = nullptr;
    }
    registered_ = false;
}

bool HotkeyManager::Restore(UINT click_key, UINT quick_key) {
    return RegisterCurrent(click_key, quick_key);
}

bool HotkeyManager::RegisterCurrent(UINT click_key, UINT quick_key) {
    bool click_registered = false;
    if (!IsMouseKey(click_key)) {
        if (!RegisterHotKey(window_, kClickHotkeyId, MOD_NOREPEAT, click_key)) return false;
        click_registered = true;
    }
    if (!IsMouseKey(quick_key) && !RegisterHotKey(window_, kQuickHotkeyId, MOD_NOREPEAT, quick_key)) {
        if (click_registered) UnregisterHotKey(window_, kClickHotkeyId);
        return false;
    }
    if (IsMouseKey(click_key) || IsMouseKey(quick_key)) {
        active_mouse_manager_ = this;
        mouse_hook_ = SetWindowsHookExW(WH_MOUSE_LL, &HotkeyManager::MouseHookProc,
                                        GetModuleHandleW(nullptr), 0);
        if (!mouse_hook_) {
            if (click_registered) UnregisterHotKey(window_, kClickHotkeyId);
            if (!IsMouseKey(quick_key)) UnregisterHotKey(window_, kQuickHotkeyId);
            active_mouse_manager_ = nullptr;
            return false;
        }
    }
    click_key_ = click_key;
    quick_key_ = quick_key;
    registered_ = true;
    return true;
}

bool HotkeyManager::IsMouseKey(UINT key) {
    return key == VK_XBUTTON1 || key == VK_XBUTTON2;
}

LRESULT CALLBACK HotkeyManager::MouseHookProc(int code, WPARAM wparam, LPARAM lparam) {
    if (code == HC_ACTION && wparam == WM_XBUTTONDOWN && active_mouse_manager_) {
        const auto* mouse = reinterpret_cast<MSLLHOOKSTRUCT*>(lparam);
        const WORD button = HIWORD(mouse->mouseData);
        const UINT key = button == XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2;
        int id = 0;
        if (key == active_mouse_manager_->click_key_) id = kClickHotkeyId;
        else if (key == active_mouse_manager_->quick_key_) id = kQuickHotkeyId;
        if (id != 0) {
            PostMessageW(active_mouse_manager_->window_, WM_HOTKEY, static_cast<WPARAM>(id), 0);
        }
    }
    return CallNextHookEx(nullptr, code, wparam, lparam);
}

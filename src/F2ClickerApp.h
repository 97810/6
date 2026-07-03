#pragma once

#include <windows.h>
#include <shellapi.h>
#include <string>
#include <vector>

#include "AudioPlayer.h"
#include "ClickController.h"
#include "ConfigManager.h"
#include "HotkeyManager.h"
#include "TimerOverlay.h"

class F2ClickerApp {
public:
    explicit F2ClickerApp(HINSTANCE instance);
    int Run(int show_command);

private:
    struct KeyOption { UINT vk; const wchar_t* name; };

    static constexpr UINT kTrayMessage = WM_APP + 1;
    static constexpr UINT_PTR kQuickClickTimerId = 3001;
    static constexpr int kButtonApply = 2001;
    static constexpr int kButtonReset = 2002;
    static constexpr int kButtonExit = 2003;
    static constexpr int kComboClick = 2101;
    static constexpr int kComboQuick = 2102;

    HINSTANCE instance_ = nullptr;
    HWND main_window_ = nullptr;
    HWND label_current_click_ = nullptr;
    HWND label_current_quick_ = nullptr;
    HWND label_coord_ = nullptr;
    HWND label_handle_ = nullptr;
    HWND label_title_ = nullptr;
    HWND label_info_ = nullptr;
    HWND combo_click_ = nullptr;
    HWND combo_quick_ = nullptr;
    bool tray_added_ = false;

    AppConfig config_{};
    ConfigManager config_manager_;
    HotkeyManager hotkeys_;
    ClickController click_controller_;
    TimerOverlay timer_overlay_;
    AudioPlayer audio_player_;
    std::vector<KeyOption> key_options_;

    static LRESULT CALLBACK StaticWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
    LRESULT WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
    bool RegisterWindowClass();
    bool CreateMainWindow(int show_command);
    void CreateControls();
    void BuildKeyOptions();
    void PopulateKeyCombos();
    void ApplyHotkeySettings();
    void OnClickHotkey();
    void OnQuickHotkey();
    bool PerformClick();
    void ResetBinding();
    void UpdateBindingLabels();
    void UpdateHotkeyLabels();
    void UpdateStatus(const std::wstring& text);
    int FindKeyIndex(UINT vk) const;
    UINT SelectedKey(HWND combo) const;
    std::wstring KeyName(UINT vk) const;
    static std::wstring FormatWindowHandle(HWND hwnd);
    void AddTrayIcon();
    void RemoveTrayIcon();
    void ShowTrayMenu();
};

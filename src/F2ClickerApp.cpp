#include "F2ClickerApp.h"

#include <cstdint>
#include <sstream>

namespace {
constexpr wchar_t kWindowClassName[] = L"F2NetworkSwitchRebuildWindow";

HWND CreateLabel(HWND parent, const wchar_t* text, int x, int y, int width, int height) {
    return CreateWindowExW(0, L"STATIC", text, WS_CHILD | WS_VISIBLE,
                           x, y, width, height, parent, nullptr, GetModuleHandleW(nullptr), nullptr);
}

HWND CreateButton(HWND parent, const wchar_t* text, int id, int x, int y, int width, int height) {
    return CreateWindowExW(0, L"BUTTON", text, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                           x, y, width, height, parent,
                           reinterpret_cast<HMENU>(static_cast<intptr_t>(id)),
                           GetModuleHandleW(nullptr), nullptr);
}
}

F2ClickerApp::F2ClickerApp(HINSTANCE instance)
    : instance_(instance), timer_overlay_(instance) {
    BuildKeyOptions();
    config_ = config_manager_.Load();
    if (FindKeyIndex(config_.click_key) < 0) config_.click_key = VK_F2;
    if (FindKeyIndex(config_.quick_key) < 0 || config_.quick_key == config_.click_key) {
        config_.quick_key = config_.click_key == VK_F3 ? VK_F2 : VK_F3;
    }
}

int F2ClickerApp::Run(int show_command) {
    if (!RegisterWindowClass() || !CreateMainWindow(show_command)) {
        MessageBoxW(nullptr, L"窗口创建失败。", L"热键点击器", MB_ICONERROR | MB_OK);
        return 1;
    }
    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return static_cast<int>(message.wParam);
}

bool F2ClickerApp::RegisterWindowClass() {
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = &F2ClickerApp::StaticWindowProc;
    wc.hInstance = instance_;
    wc.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = kWindowClassName;
    wc.hIconSm = LoadIconW(nullptr, IDI_APPLICATION);
    return RegisterClassExW(&wc) != 0;
}

bool F2ClickerApp::CreateMainWindow(int show_command) {
    main_window_ = CreateWindowExW(0, kWindowClassName, L"热键点击器",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 570, 410, nullptr, nullptr, instance_, this);
    if (!main_window_) return false;
    ShowWindow(main_window_, show_command);
    UpdateWindow(main_window_);
    return true;
}

LRESULT CALLBACK F2ClickerApp::StaticWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    F2ClickerApp* app = nullptr;
    if (message == WM_NCCREATE) {
        const auto* create = reinterpret_cast<CREATESTRUCTW*>(lparam);
        app = static_cast<F2ClickerApp*>(create->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
    } else {
        app = reinterpret_cast<F2ClickerApp*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }
    return app ? app->WindowProc(hwnd, message, wparam, lparam)
               : DefWindowProcW(hwnd, message, wparam, lparam);
}

LRESULT F2ClickerApp::WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
    case WM_CREATE: {
        CreateControls();
        hotkeys_.SetWindow(hwnd);
        timer_overlay_.Initialize();
        AddTrayIcon();
        std::wstring error;
        if (hotkeys_.Register(config_.click_key, config_.quick_key, error)) {
            UpdateStatus(L"请把鼠标放到目标位置，按点击键进行绑定。");
        } else {
            UpdateStatus(error);
        }
        return 0;
    }
    case WM_HOTKEY:
        if (static_cast<int>(wparam) == HotkeyManager::kClickHotkeyId) OnClickHotkey();
        else if (static_cast<int>(wparam) == HotkeyManager::kQuickHotkeyId) OnQuickHotkey();
        return 0;
    case WM_TIMER:
        if (wparam == kQuickClickTimerId) {
            KillTimer(hwnd, kQuickClickTimerId);
            PerformClick();
            return 0;
        }
        break;
    case WM_COMMAND:
        switch (LOWORD(wparam)) {
        case kButtonApply: ApplyHotkeySettings(); return 0;
        case kButtonReset: ResetBinding(); return 0;
        case kButtonExit: DestroyWindow(hwnd); return 0;
        default: break;
        }
        break;
    case kTrayMessage:
        if (lparam == WM_RBUTTONUP) { ShowTrayMenu(); return 0; }
        if (lparam == WM_LBUTTONDBLCLK) {
            ShowWindow(hwnd, SW_RESTORE);
            SetForegroundWindow(hwnd);
            return 0;
        }
        break;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        KillTimer(hwnd, kQuickClickTimerId);
        timer_overlay_.Hide();
        hotkeys_.Unregister();
        RemoveTrayIcon();
        PostQuitMessage(0);
        return 0;
    default:
        break;
    }
    return DefWindowProcW(hwnd, message, wparam, lparam);
}

void F2ClickerApp::CreateControls() {
    label_current_click_ = CreateLabel(main_window_, L"当前点击键：", 24, 18, 240, 24);
    label_current_quick_ = CreateLabel(main_window_, L"当前快速键：", 290, 18, 240, 24);
    CreateLabel(main_window_, L"点击键：", 24, 58, 90, 24);
    combo_click_ = CreateWindowExW(0, L"COMBOBOX", nullptr,
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        105, 54, 130, 250, main_window_,
        reinterpret_cast<HMENU>(static_cast<intptr_t>(kComboClick)), instance_, nullptr);
    CreateLabel(main_window_, L"快速键：", 265, 58, 90, 24);
    combo_quick_ = CreateWindowExW(0, L"COMBOBOX", nullptr,
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        346, 54, 130, 250, main_window_,
        reinterpret_cast<HMENU>(static_cast<intptr_t>(kComboQuick)), instance_, nullptr);
    CreateButton(main_window_, L"应用热键", kButtonApply, 480, 53, 66, 27);

    label_coord_ = CreateLabel(main_window_, L"绑定坐标：未绑定", 24, 105, 510, 24);
    label_handle_ = CreateLabel(main_window_, L"窗口句柄：未绑定", 24, 137, 510, 24);
    label_title_ = CreateLabel(main_window_, L"窗口标题：未绑定", 24, 169, 510, 24);
    label_info_ = CreateLabel(main_window_, L"状态：", 24, 201, 510, 42);
    CreateLabel(main_window_, L"点击键：绑定 / 开始计时 / 结束并点击；快速键：0.5 秒后点击", 24, 250, 510, 24);
    CreateLabel(main_window_, L"后台点击模式：已固定启用（不会移动真实鼠标）", 24, 282, 350, 26);
    CreateButton(main_window_, L"重置绑定", kButtonReset, 356, 325, 90, 30);
    CreateButton(main_window_, L"退出", kButtonExit, 456, 325, 90, 30);
    PopulateKeyCombos();
    UpdateHotkeyLabels();
}

void F2ClickerApp::BuildKeyOptions() {
    static constexpr const wchar_t* function_names[] = {
        L"F1", L"F2", L"F3", L"F4", L"F5", L"F6", L"F7", L"F8", L"F9", L"F10", L"F11", L"F12"
    };
    for (UINT i = 0; i < 12; ++i) key_options_.push_back({VK_F1 + i, function_names[i]});
    static constexpr const wchar_t* letter_names[] = {
        L"A",L"B",L"C",L"D",L"E",L"F",L"G",L"H",L"I",L"J",L"K",L"L",L"M",
        L"N",L"O",L"P",L"Q",L"R",L"S",L"T",L"U",L"V",L"W",L"X",L"Y",L"Z"
    };
    for (UINT i = 0; i < 26; ++i) key_options_.push_back({static_cast<UINT>('A') + i, letter_names[i]});
    static constexpr const wchar_t* digit_names[] = {L"0",L"1",L"2",L"3",L"4",L"5",L"6",L"7",L"8",L"9"};
    for (UINT i = 0; i < 10; ++i) key_options_.push_back({static_cast<UINT>('0') + i, digit_names[i]});
    key_options_.push_back({VK_XBUTTON1, L"鼠标侧键 1"});
    key_options_.push_back({VK_XBUTTON2, L"鼠标侧键 2"});
}

void F2ClickerApp::PopulateKeyCombos() {
    for (const auto& option : key_options_) {
        SendMessageW(combo_click_, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(option.name));
        SendMessageW(combo_quick_, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(option.name));
    }
    SendMessageW(combo_click_, CB_SETCURSEL, static_cast<WPARAM>(FindKeyIndex(config_.click_key)), 0);
    SendMessageW(combo_quick_, CB_SETCURSEL, static_cast<WPARAM>(FindKeyIndex(config_.quick_key)), 0);
}

void F2ClickerApp::ApplyHotkeySettings() {
    const UINT click_key = SelectedKey(combo_click_);
    const UINT quick_key = SelectedKey(combo_quick_);
    std::wstring error;
    if (!hotkeys_.Register(click_key, quick_key, error)) {
        MessageBoxW(main_window_, error.c_str(), L"热键设置", MB_OK | MB_ICONWARNING);
        UpdateStatus(error);
        return;
    }
    config_.click_key = click_key;
    config_.quick_key = quick_key;
    const bool saved = config_manager_.Save(config_);
    UpdateHotkeyLabels();
    UpdateStatus(saved ? L"热键已应用并保存。" : L"热键已应用，但配置文件保存失败。");
}

void F2ClickerApp::OnClickHotkey() {
    KillTimer(main_window_, kQuickClickTimerId);
    if (!click_controller_.HasTarget()) {
        std::wstring error;
        if (click_controller_.BindUnderMouse(error)) {
            UpdateBindingLabels();
            UpdateStatus(L"绑定成功。再次按点击键将立即点击并开始计时。");
        } else {
            UpdateStatus(error);
        }
        return;
    }

    // Every click-key press after binding performs the click immediately.
    // The overlay state determines whether this press starts or ends timing.
    if (!PerformClick()) return;

    if (!timer_overlay_.IsVisible()) {
        timer_overlay_.Show();
        audio_player_.PlayStart();
        UpdateStatus(L"已发送后台点击，计时开始。");
    } else {
        timer_overlay_.Hide();
        audio_player_.PlayEnd();
        UpdateStatus(L"已发送后台点击，计时结束。");
    }
}

void F2ClickerApp::OnQuickHotkey() {
    if (!click_controller_.HasTarget()) {
        UpdateStatus(L"请先用点击键绑定坐标。 ");
        MessageBoxW(main_window_, L"尚未绑定坐标，请先把鼠标放到目标位置并按点击键。",
                    L"快速点击", MB_OK | MB_ICONINFORMATION);
        return;
    }

    // A bound target alone is not enough: quick click is only enabled while
    // the timing overlay is active.
    if (!timer_overlay_.IsVisible()) {
        UpdateStatus(L"请先按点击键开启计时，再使用快速键。 ");
        return;
    }

    timer_overlay_.Hide();
    audio_player_.PlayEnd();
    KillTimer(main_window_, kQuickClickTimerId);
    if (SetTimer(main_window_, kQuickClickTimerId, 500, nullptr) == 0) {
        UpdateStatus(L"快速点击定时器启动失败。 ");
        return;
    }
    UpdateStatus(L"快速键触发，0.5 秒后点击。 ");
}

bool F2ClickerApp::PerformClick() {
    std::wstring error;
    if (click_controller_.ClickBoundTarget(error)) {
        UpdateStatus(L"已发送后台点击。");
        return true;
    } else {
        UpdateBindingLabels();
        UpdateStatus(error);
        return false;
    }
}

void F2ClickerApp::ResetBinding() {
    KillTimer(main_window_, kQuickClickTimerId);
    timer_overlay_.Hide();
    click_controller_.Reset();
    UpdateBindingLabels();
    UpdateStatus(L"绑定已重置，请把鼠标放到目标位置并按点击键。 ");
}

void F2ClickerApp::UpdateBindingLabels() {
    if (!click_controller_.HasTarget()) {
        SetWindowTextW(label_coord_, L"绑定坐标：未绑定");
        SetWindowTextW(label_handle_, L"窗口句柄：未绑定");
        SetWindowTextW(label_title_, L"窗口标题：未绑定");
        return;
    }
    const POINT client_point = click_controller_.TargetClientPoint();
    const POINT screen_point = click_controller_.TargetScreenPoint();
    std::wostringstream coord;
    coord << L"绑定坐标：屏幕 " << screen_point.x << L", " << screen_point.y
          << L"；控件内 " << client_point.x << L", " << client_point.y;
    SetWindowTextW(label_coord_, coord.str().c_str());
    const std::wstring handles = FormatWindowHandle(click_controller_.TargetWindow())
        + L"；根 " + FormatWindowHandle(click_controller_.RootWindow()).substr(5);
    SetWindowTextW(label_handle_, handles.c_str());
    const std::wstring title = L"窗口标题：" + (click_controller_.WindowTitle().empty()
        ? std::wstring(L"（无标题）") : click_controller_.WindowTitle());
    SetWindowTextW(label_title_, title.c_str());
}

void F2ClickerApp::UpdateHotkeyLabels() {
    const std::wstring click = L"当前点击键：" + KeyName(config_.click_key);
    const std::wstring quick = L"当前快速键：" + KeyName(config_.quick_key);
    SetWindowTextW(label_current_click_, click.c_str());
    SetWindowTextW(label_current_quick_, quick.c_str());
}

void F2ClickerApp::UpdateStatus(const std::wstring& text) {
    if (label_info_) SetWindowTextW(label_info_, (L"状态：" + text).c_str());
}

int F2ClickerApp::FindKeyIndex(UINT vk) const {
    for (size_t i = 0; i < key_options_.size(); ++i) {
        if (key_options_[i].vk == vk) return static_cast<int>(i);
    }
    return -1;
}

UINT F2ClickerApp::SelectedKey(HWND combo) const {
    const LRESULT selected = SendMessageW(combo, CB_GETCURSEL, 0, 0);
    if (selected == CB_ERR || static_cast<size_t>(selected) >= key_options_.size()) return 0;
    return key_options_[static_cast<size_t>(selected)].vk;
}

std::wstring F2ClickerApp::KeyName(UINT vk) const {
    const int index = FindKeyIndex(vk);
    return index >= 0 ? key_options_[static_cast<size_t>(index)].name : L"未知";
}

std::wstring F2ClickerApp::FormatWindowHandle(HWND hwnd) {
    std::wostringstream text;
    text << L"窗口句柄：0x" << std::hex << reinterpret_cast<std::uintptr_t>(hwnd);
    return text.str();
}

void F2ClickerApp::AddTrayIcon() {
    NOTIFYICONDATAW data{};
    data.cbSize = sizeof(data);
    data.hWnd = main_window_;
    data.uID = 1;
    data.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    data.uCallbackMessage = kTrayMessage;
    data.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    lstrcpynW(data.szTip, L"热键点击器", ARRAYSIZE(data.szTip));
    tray_added_ = Shell_NotifyIconW(NIM_ADD, &data) != FALSE;
}

void F2ClickerApp::RemoveTrayIcon() {
    if (!tray_added_) return;
    NOTIFYICONDATAW data{};
    data.cbSize = sizeof(data);
    data.hWnd = main_window_;
    data.uID = 1;
    Shell_NotifyIconW(NIM_DELETE, &data);
    tray_added_ = false;
}

void F2ClickerApp::ShowTrayMenu() {
    HMENU menu = CreatePopupMenu();
    if (!menu) return;
    AppendMenuW(menu, MF_STRING, kButtonExit, L"退出");
    POINT cursor{};
    GetCursorPos(&cursor);
    SetForegroundWindow(main_window_);
    TrackPopupMenu(menu, TPM_RIGHTBUTTON, cursor.x, cursor.y, 0, main_window_, nullptr);
    DestroyMenu(menu);
}

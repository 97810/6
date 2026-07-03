#include "ClickController.h"

#include <array>

bool ClickController::BindUnderMouse(std::wstring& error) {
    POINT screen_point{};
    if (!GetCursorPos(&screen_point)) {
        error = L"获取鼠标位置失败。";
        return false;
    }

    HWND child_window = WindowFromPoint(screen_point);
    if (!child_window) {
        error = L"未找到鼠标所在窗口。";
        return false;
    }
    HWND root_window = GetAncestor(child_window, GA_ROOT);
    if (!root_window) root_window = child_window;

    POINT child_client_point = screen_point;
    if (!ScreenToClient(child_window, &child_client_point)) {
        error = L"子窗口坐标转换失败。";
        return false;
    }
    POINT root_client_point = screen_point;
    if (!ScreenToClient(root_window, &root_client_point)) {
        error = L"顶层窗口坐标转换失败。";
        return false;
    }

    child_window_ = child_window;
    root_window_ = root_window;
    screen_point_ = screen_point;
    child_client_point_ = child_client_point;
    root_client_point_ = root_client_point;
    window_title_ = ReadWindowTitle(root_window_);
    if (window_title_.empty() && child_window_ != root_window_) {
        window_title_ = ReadWindowTitle(child_window_);
    }
    has_target_ = true;
    return true;
}

bool ClickController::ClickBoundTarget(std::wstring& error) {
    // Match the verified reference executable: hotkey clicks always use the
    // background message path. ClickRealMouse remains available internally as
    // a fallback implementation, but is never selected by the hotkey path.
    return ClickBackground(error);
}

bool ClickController::ClickBackground(std::wstring& error) {
    HWND target = nullptr;
    POINT client_point{};
    if (!ResolveTarget(target, client_point, error)) return false;

    const LPARAM position = MAKELPARAM(client_point.x, client_point.y);
    // Match the verified reference executable exactly: send button-down and
    // button-up to the bound HWND using its saved client coordinates. Do not
    // inject WM_MOUSEMOVE or any system-wide input.
    // Call both even if down fails, so a partially delivered sequence does
    // not leave the target believing that the left button is still held down.
    const bool down_sent = PostMessageW(target, WM_LBUTTONDOWN, MK_LBUTTON, position) != FALSE;
    const bool up_sent = PostMessageW(target, WM_LBUTTONUP, 0, position) != FALSE;
    if (!down_sent || !up_sent) {
        error = L"后台点击可能不被该程序支持，可尝试切换真实点击模式。";
        return false;
    }
    return true;
}

bool ClickController::ClickRealMouse(std::wstring& error) {
    HWND target = nullptr;
    POINT client_point{};
    if (!ResolveTarget(target, client_point, error)) return false;

    POINT screen_point = client_point;
    if (!ClientToScreen(target, &screen_point)) {
        error = L"目标坐标转换失败。";
        return false;
    }
    if (!SetCursorPos(screen_point.x, screen_point.y)) {
        error = L"移动鼠标失败。";
        return false;
    }

    INPUT inputs[2]{};
    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
    if (SendInput(2, inputs, sizeof(INPUT)) != 2) {
        error = L"真实鼠标点击失败。";
        return false;
    }
    return true;
}

bool ClickController::ResolveTarget(HWND& target, POINT& client_point, std::wstring& error) {
    if (!has_target_) {
        error = L"请先绑定坐标。";
        return false;
    }
    if (child_window_ && IsWindow(child_window_)) {
        target = child_window_;
        client_point = child_client_point_;
        return true;
    }
    if (root_window_ && IsWindow(root_window_)) {
        target = root_window_;
        client_point = root_client_point_;
        return true;
    }

    Reset();
    error = L"绑定窗口已失效，请重新绑定。";
    return false;
}

std::wstring ClickController::ReadWindowTitle(HWND window) {
    std::array<wchar_t, 512> title{};
    const int length = GetWindowTextW(window, title.data(), static_cast<int>(title.size()));
    return length > 0 ? std::wstring(title.data(), static_cast<size_t>(length)) : std::wstring{};
}

void ClickController::Reset() {
    child_window_ = nullptr;
    root_window_ = nullptr;
    screen_point_ = POINT{0, 0};
    child_client_point_ = POINT{0, 0};
    root_client_point_ = POINT{0, 0};
    window_title_.clear();
    has_target_ = false;
}

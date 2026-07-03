#include "TimerOverlay.h"

#include <iomanip>
#include <sstream>

namespace {
constexpr wchar_t kOverlayClass[] = L"F2ClickerTimerOverlay";
constexpr int kWidth = 132;
constexpr int kHeight = 58;
}

TimerOverlay::TimerOverlay(HINSTANCE instance) : instance_(instance) {}

TimerOverlay::~TimerOverlay() {
    if (window_) DestroyWindow(window_);
}

bool TimerOverlay::Initialize() {
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = &TimerOverlay::StaticWindowProc;
    wc.hInstance = instance_;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    // WM_PAINT fills the complete client area, so no class brush is needed.
    wc.hbrBackground = nullptr;
    wc.lpszClassName = kOverlayClass;
    RegisterClassExW(&wc);

    window_ = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        kOverlayClass, L"计时", WS_POPUP,
        0, 0, kWidth, kHeight, nullptr, nullptr, instance_, this);
    return window_ != nullptr;
}

void TimerOverlay::Show() {
    if (!window_) return;
    started_at_ = GetTickCount64();
    const int x = GetSystemMetrics(SM_CXSCREEN) - kWidth - 20;
    SetWindowPos(window_, HWND_TOPMOST, x, 20, kWidth, kHeight, SWP_NOACTIVATE | SWP_SHOWWINDOW);
    SetTimer(window_, kRefreshTimerId, 100, nullptr);
    visible_ = true;
    InvalidateRect(window_, nullptr, TRUE);
}

void TimerOverlay::Hide() {
    if (!window_) return;
    KillTimer(window_, kRefreshTimerId);
    ShowWindow(window_, SW_HIDE);
    visible_ = false;
}

LRESULT CALLBACK TimerOverlay::StaticWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    TimerOverlay* overlay = nullptr;
    if (message == WM_NCCREATE) {
        const auto* create = reinterpret_cast<CREATESTRUCTW*>(lparam);
        overlay = static_cast<TimerOverlay*>(create->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(overlay));
    } else {
        overlay = reinterpret_cast<TimerOverlay*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }
    return overlay ? overlay->WindowProc(hwnd, message, wparam, lparam)
                   : DefWindowProcW(hwnd, message, wparam, lparam);
}

LRESULT TimerOverlay::WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
    case WM_TIMER:
        if (wparam == kRefreshTimerId) InvalidateRect(hwnd, nullptr, TRUE);
        return 0;
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT:
        Paint();
        return 0;
    default:
        return DefWindowProcW(hwnd, message, wparam, lparam);
    }
}

void TimerOverlay::Paint() {
    PAINTSTRUCT ps{};
    HDC dc = BeginPaint(window_, &ps);
    RECT rect{};
    GetClientRect(window_, &rect);
    HBRUSH background = CreateSolidBrush(RGB(35, 38, 45));
    FillRect(dc, &rect, background);
    DeleteObject(background);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(245, 245, 245));
    HFONT font = CreateFontW(30, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    HGDIOBJ old_font = SelectObject(dc, font);
    const double seconds = static_cast<double>(GetTickCount64() - started_at_) / 1000.0;
    std::wostringstream text;
    text << std::fixed << std::setprecision(1) << seconds << L"s";
    DrawTextW(dc, text.str().c_str(), -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(dc, old_font);
    DeleteObject(font);
    EndPaint(window_, &ps);
}

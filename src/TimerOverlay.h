#pragma once

#include <windows.h>

class TimerOverlay {
public:
    explicit TimerOverlay(HINSTANCE instance);
    ~TimerOverlay();

    bool Initialize();
    void Show();
    void Hide();
    bool IsVisible() const { return visible_; }

private:
    static constexpr UINT_PTR kRefreshTimerId = 1;
    static LRESULT CALLBACK StaticWindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
    LRESULT WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
    void Paint();

    HINSTANCE instance_ = nullptr;
    HWND window_ = nullptr;
    ULONGLONG started_at_ = 0;
    bool visible_ = false;
};

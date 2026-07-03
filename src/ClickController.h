#pragma once

#include <windows.h>
#include <string>

class ClickController {
public:
    bool BindUnderMouse(std::wstring& error);
    bool ClickBoundTarget(std::wstring& error);
    void Reset();

    bool HasTarget() const { return has_target_; }
    HWND TargetWindow() const { return child_window_; }
    HWND RootWindow() const { return root_window_; }
    POINT TargetScreenPoint() const { return screen_point_; }
    POINT TargetClientPoint() const { return child_client_point_; }
    const std::wstring& WindowTitle() const { return window_title_; }

private:
    bool ClickBackground(std::wstring& error);
    bool ClickRealMouse(std::wstring& error);
    bool ResolveTarget(HWND& target, POINT& client_point, std::wstring& error);
    static std::wstring ReadWindowTitle(HWND window);

    HWND child_window_ = nullptr;
    HWND root_window_ = nullptr;
    POINT screen_point_{0, 0};
    POINT child_client_point_{0, 0};
    POINT root_client_point_{0, 0};
    std::wstring window_title_;
    bool has_target_ = false;
};

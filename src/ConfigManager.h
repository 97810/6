#pragma once

#include <windows.h>
#include <string>

struct AppConfig {
    UINT click_key = VK_F2;
    UINT quick_key = VK_F3;
};

class ConfigManager {
public:
    ConfigManager();
    AppConfig Load() const;
    bool Save(const AppConfig& config) const;
    const std::wstring& Path() const { return path_; }

private:
    std::wstring path_;
};

#include "ConfigManager.h"
#include "RuntimePaths.h"

#include <filesystem>

ConfigManager::ConfigManager()
    : path_(RuntimePaths::FromExecutable(L"config.ini").wstring()) {}

AppConfig ConfigManager::Load() const {
    AppConfig config;
    config.click_key = GetPrivateProfileIntW(L"Hotkeys", L"ClickKey", VK_F2, path_.c_str());
    config.quick_key = GetPrivateProfileIntW(L"Hotkeys", L"QuickKey", VK_F3, path_.c_str());
    return config;
}

bool ConfigManager::Save(const AppConfig& config) const {
    const std::wstring click = std::to_wstring(config.click_key);
    const std::wstring quick = std::to_wstring(config.quick_key);
    return WritePrivateProfileStringW(L"Hotkeys", L"ClickKey", click.c_str(), path_.c_str()) != FALSE &&
           WritePrivateProfileStringW(L"Hotkeys", L"QuickKey", quick.c_str(), path_.c_str()) != FALSE;
}

#include "RuntimePaths.h"

#include <windows.h>

#include <stdexcept>
#include <string>
#include <vector>

namespace RuntimePaths {

std::filesystem::path ExecutableDirectory() {
    std::vector<wchar_t> buffer(1024);
    for (;;) {
        const DWORD length = GetModuleFileNameW(
            nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (length == 0) {
            throw std::runtime_error("GetModuleFileNameW failed");
        }
        if (static_cast<size_t>(length) < buffer.size() - 1) {
            return std::filesystem::path(std::wstring(buffer.data(), length)).parent_path();
        }
        buffer.resize(buffer.size() * 2);
    }
}

std::filesystem::path FromExecutable(const std::filesystem::path& relative_path) {
    return ExecutableDirectory() / relative_path;
}

} // namespace RuntimePaths

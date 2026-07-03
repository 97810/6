#pragma once

#include <filesystem>

namespace RuntimePaths {

std::filesystem::path ExecutableDirectory();
std::filesystem::path FromExecutable(const std::filesystem::path& relative_path);

} // namespace RuntimePaths

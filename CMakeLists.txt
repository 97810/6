cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0091 NEW)
project(F2NetworkSwitchRebuild LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Use the static MSVC runtime: /MT in Release and /MTd in Debug.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_executable(F2NetworkSwitch WIN32
    src/main.cpp
    src/F2ClickerApp.cpp
    src/F2ClickerApp.h
    src/HotkeyManager.cpp
    src/HotkeyManager.h
    src/ConfigManager.cpp
    src/ConfigManager.h
    src/ClickController.cpp
    src/ClickController.h
    src/TimerOverlay.cpp
    src/TimerOverlay.h
    src/AudioPlayer.cpp
    src/AudioPlayer.h
    src/RuntimePaths.cpp
    src/RuntimePaths.h
    resources/app.rc
)

set_target_properties(F2NetworkSwitch PROPERTIES OUTPUT_NAME "F2一键断网")

target_compile_definitions(F2NetworkSwitch PRIVATE UNICODE _UNICODE NOMINMAX)
target_include_directories(F2NetworkSwitch PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/resources)

target_link_libraries(F2NetworkSwitch PRIVATE user32 shell32 gdi32 comctl32 winmm)

# Runtime code loads these files from <exe directory>/lib.
add_custom_command(TARGET F2NetworkSwitch POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:F2NetworkSwitch>/lib"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/Start.mp3"
            "$<TARGET_FILE_DIR:F2NetworkSwitch>/lib/start.mp3"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/end.mp3"
            "$<TARGET_FILE_DIR:F2NetworkSwitch>/lib/end.mp3"
)

# Build with: cmake --build build --config Release --target package_release
set(DELIVERY_DIR "${CMAKE_BINARY_DIR}/dist/F2一键断网_v1.0")
add_custom_target(package_release
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${DELIVERY_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DELIVERY_DIR}/lib"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:F2NetworkSwitch>"
            "${DELIVERY_DIR}/F2一键断网.exe"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/Start.mp3"
            "${DELIVERY_DIR}/lib/start.mp3"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/lib/end.mp3"
            "${DELIVERY_DIR}/lib/end.mp3"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/packaging/使用说明.txt"
            "${DELIVERY_DIR}/使用说明.txt"
    DEPENDS F2NetworkSwitch
    COMMENT "Creating release delivery folder: ${DELIVERY_DIR}"
    VERBATIM
)

if (MSVC)
    target_compile_options(F2NetworkSwitch PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(F2NetworkSwitch PRIVATE -Wall -Wextra -Wpedantic)
endif()

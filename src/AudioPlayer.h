#pragma once

#include <string>

class AudioPlayer {
public:
    AudioPlayer();
    ~AudioPlayer();

    bool PlayStart();
    bool PlayEnd();

private:
    bool Play(const std::wstring& path, const wchar_t* alias);
    static void Close(const wchar_t* alias);

    std::wstring start_path_;
    std::wstring end_path_;
};

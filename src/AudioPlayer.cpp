#include "AudioPlayer.h"
#include "RuntimePaths.h"

#include <windows.h>
#include <mmsystem.h>
#include <filesystem>

AudioPlayer::AudioPlayer() {
    start_path_ = RuntimePaths::FromExecutable(L"lib/start.mp3").wstring();
    end_path_ = RuntimePaths::FromExecutable(L"lib/end.mp3").wstring();
}

AudioPlayer::~AudioPlayer() {
    Close(L"clicker_start");
    Close(L"clicker_end");
}

bool AudioPlayer::PlayStart() {
    Close(L"clicker_end");
    return Play(start_path_, L"clicker_start");
}

bool AudioPlayer::PlayEnd() {
    Close(L"clicker_start");
    return Play(end_path_, L"clicker_end");
}

bool AudioPlayer::Play(const std::wstring& path, const wchar_t* alias) {
    if (GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES) return false;
    Close(alias);
    const std::wstring open = L"open \"" + path + L"\" type mpegvideo alias " + alias;
    if (mciSendStringW(open.c_str(), nullptr, 0, nullptr) != 0) return false;
    const std::wstring play = std::wstring(L"play ") + alias + L" from 0";
    return mciSendStringW(play.c_str(), nullptr, 0, nullptr) == 0;
}

void AudioPlayer::Close(const wchar_t* alias) {
    const std::wstring command = std::wstring(L"close ") + alias;
    mciSendStringW(command.c_str(), nullptr, 0, nullptr);
}

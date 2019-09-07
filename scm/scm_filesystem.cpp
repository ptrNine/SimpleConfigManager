#include "scm_filesystem.hpp"

#include <string>
#include <array>
#include <iostream>
#include <string_view>


//////////////////////////// UNIX
#ifdef __unix__

#include <cstring>
#include <unistd.h>
#include <sys/stat.h>

#define DE_PATH_MAX 8192

int makeDirAbort(const std::string& path) {
    return scm_fs_dtls::_makeDirAbort(path, strerror(errno));
}

int scm_fs_dtls::_recursiveMakeDir(const std::string_view& path) {
    if (path.empty() || path == "." || path == "/")
        return 0;


    auto findPos = path.rfind('/');
    if (findPos == std::string_view::npos)
        return 0;

    auto parent = path.substr(0, findPos);

    if (_recursiveMakeDir(parent) == -1 && errno != EEXIST)
        return -1;

    return (mkdir(std::string(path).c_str(), 0777) == -1 && errno != EEXIST) ?
           makeDirAbort(std::string(path)) : 0;
}


auto scm_fs_dtls::_getExeLocation() -> std::string {
    auto result = std::array<char, DE_PATH_MAX>();
    auto count  = readlink( "/proc/self/exe", result.data(), DE_PATH_MAX);

    if (count < 0)
        count = 0;

    // Drop exe name
    for (auto p = result.crbegin() + DE_PATH_MAX - count; p != result.crend() && *p != '/'; ++p)
        --count;

    return { result.data(), static_cast<std::size_t>(count) };
}

#undef DE_PATH_MAX

///////////////////////////// WINDOWS
#elif _WIN32

#include <codecvt>
#include <locale>
#include <algorithm>
#include <windows.h>
#define DE_PATH_MAX 8192

std::string GetLastErrorAsString(int rc) {
    if(rc == 0)
        return {};

    LPSTR messageBuffer = nullptr;
    std::size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, rc, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    auto message = std::string(messageBuffer, size);

    LocalFree(messageBuffer);

    return message;
}

int makeDirAbort(const std::string& path, int rc) {
    return scm_fs_dtls::_makeDirAbort(path, GetLastErrorAsString(rc).c_str());
}

int scm_fs_dtls::_recursiveMakeDir(const std::string_view& path) {
    if (path.empty() || path == "." || path == "/")
            return 0;

    auto findPos = path.rfind('/');
    if (findPos == std::string_view::npos)
        return 0;

    auto parent = path.substr(0, findPos);

    if (_recursiveMakeDir(parent) == -1 && GetLastError() != ERROR_ALREADY_EXISTS)
        return -1;

	CreateDirectory(std::string(path).c_str(), nullptr);
	auto rc = GetLastError();

    return (rc != ERROR_ALREADY_EXISTS && rc != 0) ?
    makeDirAbort(std::string(path), static_cast<int>(rc)) : 0;

}

auto scm_fs_dtls::_getExeLocation() -> std::string {
    auto result = std::array<wchar_t, DE_PATH_MAX>();
    auto count  = GetModuleFileNameW(nullptr, result.data(), DE_PATH_MAX);
    result[count] = L'\0';

    if (count < 0)
        count = 0;

    // Drop exe name
    for (auto p = result.crbegin() + DE_PATH_MAX - count; p != result.crend() && *p != L'\\'; ++p)
        --count;

    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> convert;

    auto res = convert.to_bytes(result.data(), result.data() + count);
    std::replace(res.begin(), res.end(), '\\', '/');

    return res;
}
#undef DE_PATH_MAX


//////////////////////////////// NO SUPPORT
#else
#error "Your system doesn't supported!"
#endif
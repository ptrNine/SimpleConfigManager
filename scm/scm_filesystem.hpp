#pragma once

#include <exception>

#include "scm_config.hpp"
#include "scm_types.hpp"

/////////////////////////////////// Details ////////////////////////////////////

namespace scm_fs_dtls {
    class ScmFsException : public std::exception {
    public:
        ScmFsException(ScmString error) : _exc(std::move(error)) {}
        const char* what() const noexcept override { return _exc.data(); }

    private:
        ScmString _exc;
    };


    inline int _makeDirAbort(const std::string& path, const char* error) {
#ifdef SCM_ASSERTS
        SCM_ASSERTS("Can't create directory '%s'. %s", path.data(), error);
        return -1;
#elif defined SCM_FMT_ASSERTS
        SCM_FMT_ASSERTS("Can't create directory '{}'. {}", path, error);
        return -1;
#else
        throw ScmFsException(ScmString("Can't create directory '") + path + "'. " + error);
#endif
    }

    auto _getExeLocation   () -> std::string;
    int  _recursiveMakeDir (const std::string_view& path);

} // namespace scm_fs_dtls


#define IA inline auto


#ifdef SCM_NAMESPACE
namespace SCM_NAMESPACE {
#endif // SCM_NAMESPACE

namespace fs {
    using FsException = scm_fs_dtls::ScmFsException;

    inline auto current_path() -> ScmString {
        return scm_fs_dtls::_getExeLocation();
    }

    inline void create_dir(const std::string_view& path) {
        scm_fs_dtls::_recursiveMakeDir(path);
    }

} // namespace fs

#ifdef SCM_NAMESPACE
}
#endif // SCM_NAMESPACE


#undef IA
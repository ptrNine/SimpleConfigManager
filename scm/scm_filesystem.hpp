#pragma once

#include <exception>

#include "scm_config.hpp"
#include "scm_utils.hpp"

/////////////////////////////////// Details ////////////////////////////////////

namespace scm_fs_dtls {
    inline int _makeDirAbort(const std::string& path, const char* error) {
        SCM_EXCEPTION(scm_utils::ScmFsException, 0, ScmString("Can't create directory '") + path + "'. " + error);
    }

    auto _getExeLocation   () -> std::string;
    int  _recursiveMakeDir (const std::string_view& path);

} // namespace scm_fs_dtls


#define IA inline auto


#ifdef SCM_NAMESPACE
namespace SCM_NAMESPACE {
#endif // SCM_NAMESPACE

namespace fs {
    using FsException = scm_utils::ScmFsException;

    inline auto current_path() -> ScmString {
        return scm_fs_dtls::_getExeLocation();
    }

    inline void create_dir(const std::string_view& path) {
        scm_fs_dtls::_recursiveMakeDir(path);
    }

    inline auto default_cfg_path() -> ScmString {
        return scm_utils::append_path(current_path(), "fs.cfg");
    }

} // namespace fs

#ifdef SCM_NAMESPACE
}
#endif // SCM_NAMESPACE


#undef IA
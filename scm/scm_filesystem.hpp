#pragma once

#include <exception>

#include "scm_utils.hpp"

/////////////////////////////////// Details ////////////////////////////////////

namespace scm_fs_dtls {
    inline int _makeDirAbort(const std::string& path, const char* error) {
        SCM_EXCEPTION(SCM_NAMESPACE::ScmFsException, 0, ScmString("Can't create directory '") + path + "'. " + error);
        return 0;
    }

    auto _getExeLocation   () -> std::string;
    int  _recursiveMakeDir (const std::string_view& path);

} // namespace scm_fs_dtls

#ifndef SCM_NAMESPACE
    #define SCM_NAMESPACE scm
#endif

namespace SCM_NAMESPACE {
    namespace fs {
        /**
         * Return executable path
         * @return string with executable path
         */
        inline auto current_path() -> ScmString {
            return scm_fs_dtls::_getExeLocation();
        }

        /**
         * Recursive create dir
         * @param path - path for creation
         */
        inline void create_dir(const std::string_view &path) {
            scm_fs_dtls::_recursiveMakeDir(path);
        }

        /**
         * Return default entry config file path
         * @return string with executable dir + fs.cfg
         */
        inline auto default_cfg_path() -> ScmString {
            return append_path(current_path(), "fs.cfg");
        }

    } // namespace fs
} // namespace SCM_NAMESPACE
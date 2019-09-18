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
    auto _listFiles        (const std::string_view& path) -> std::vector<std::string>;
    auto _listDirs         (const std::string_view& path) -> std::vector<std::string>;

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
        inline void create_dir(const ScmStrView& path) {
            scm_fs_dtls::_recursiveMakeDir(path);
        }

        /**
         * List files in directory
         * @param path - path to directory
         * @return vector of files
         */
        inline auto list_files(const ScmStrView& path) -> ScmVector<ScmString> {
            auto res = scm_fs_dtls::_listFiles(path);
            return ScmVector<ScmString>(res.begin(), res.end());
        }

        /**
         * List directories in directory
         * @param path - path to directory
         * @return vector of directories
         */
        inline auto list_directories(const ScmStrView& path) -> ScmVector<ScmString> {
            auto res = scm_fs_dtls::_listDirs(path);
            return ScmVector<ScmString>(res.begin(), res.end());
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
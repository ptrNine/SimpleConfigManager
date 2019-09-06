#pragma once

#include <iostream>
#include <fstream>

#include "scm_types.hpp"

#ifdef SCM_ASSERTS
    #define SCM_EXCEPTION(EXCEPTION_TYPE, CONDITION, ...) \
        SCM_ASSERTS((CONDITION), "%s", scm_utils::str_join(__VA_ARGS__))
#elif defined SCM_FMT_ASSERTS
    #define SCM_EXCEPTION(EXCEPTION_TYPE, CONDITION, ...) \
    SCM_FMT_ASSERTS((CONDITION), "{}", scm_utils::str_join(__VA_ARGS__))
#else
    #define SCM_EXCEPTION(EXCEPTION_TYPE, CONDITION, ...)          \
        if (!(CONDITION))                                          \
            throw EXCEPTION_TYPE(scm_utils::str_join(__VA_ARGS__))
#endif

namespace scm_utils {
    /**
     * Input file stream exception
     */
    class ScmIfsException : public std::exception {
    public:
        explicit ScmIfsException(std::string error) : _exc(std::move(error)) {}
        const char* what() const noexcept override { return _exc.data(); }

    private:
        std::string _exc;
    };

    /**
     * Config exception
     */
    class CfgException : public std::exception {
    public:
        explicit CfgException(std::string error) : _exc(std::move(error)) {}
        const char* what() const noexcept override { return _exc.data(); }

    private:
        std::string _exc;
    };

    /**
     * Filesystem exception
     */
    class ScmFsException : public std::exception {
    public:
        explicit ScmFsException(std::string error) : _exc(std::move(error)) {}
        const char* what() const noexcept override { return _exc.data(); }

    private:
        std::string _exc;
    };

    /**
     * Check if type 'T' belongs to one of types 'Types'
     * @tparam T - checked type
     * @tparam Types - all valid types
     */
    template <typename T, typename ... Types>
    constexpr bool any_of = (std::is_same_v<T, Types> || ...);

    /**
     * Check if type 'T' belongs to floats
     * @tparam T - checked type
     */
    template <typename T>
    constexpr bool floats =
            std::is_floating_point_v<std::remove_const_t<std::remove_reference_t<T>>>;

    /**
     * Check if type 'T' belongs to integers
     * @tparam T - checked type
     */
    template <typename T>
    constexpr bool integers =
            std::is_integral_v<std::remove_const_t<std::remove_reference_t<T>>> && !std::is_same_v<T, bool>;

    /**
     * Check if type 'T' belongs to numbers
     * @tparam T - checked type
     */
    template <typename T>
    constexpr bool numbers = integers<T> || floats<T>;

    /**
     * Check if type 'T' is std::string, std::string_view, ScmString or StmStrView
     * @tparam T - checked type
     */
    template <typename T>
    constexpr bool is_string = any_of<T, ScmString, ScmStrView, std::string, std::string_view>;


    /**
     * Combine all accepted strings
     * @tparam ArgT - types of strings
     * @param strs - strings
     * @return New string
     */
    template <typename ... ArgT>
    static auto str_join(const ArgT& ... strs) -> std::string {
        return (std::string(strs) + ...);
    }

    /**
     * Drop the child path from string
     * @tparam S - string type
     * @param str - string with path
     * @return string with deleted child path
     */
    template <typename S>
    inline auto parent_path(const S& str) -> std::enable_if_t<is_string<S>, ScmString>
    {
        auto size = str.length();

        auto p = str.crbegin();

        if (p != str.crend() && *p == '/') {
            ++p;
            --size;
        }

        for (; p != str.crend() && *p != '/'; ++p)
            --size;

        if (p == str.crend())
            return {};

        return str.substr(0, size);
    }

    /**
     * Append path to string
     * @tparam S - first string type (also type of returned string)
     * @tparam S2 - second string type
     * @param p1 - path
     * @param str2 - appended path
     * @return new path
     */
    template <typename S, typename S2>
    inline auto append_path(const S& p1, const S2& str2) -> std::enable_if_t<is_string<S>, ScmString>
    {
        ScmString p2 = str2;
        if (p1.empty() || p2.empty())
            return p1 + p2;
        else {
            if (p1.back() == '/' && p2.front() == '/')
                return ScmString(p1 + (p2.data() + 1));
            else if (p1.back() == '/' || p2.front() == '/')
                return ScmString(p1 + p2);
            else
                return ScmString(p1 + '/' + p2);
        }
    }

    /**
     * Read whole file to string (without buffers, be careful!)
     * @tparam S - string type
     * @param name - path to file
     * @return A string contained whole file
     */
    template <typename S>
    inline auto read_file_to_string(const S& name) -> std::enable_if_t<is_string<S>, ScmString>
    {
        auto path = ScmString(name);
        auto ifs = std::ifstream(path.data(), std::ios_base::binary | std::ios_base::in);

        SCM_EXCEPTION(ScmIfsException, ifs.is_open(), "Can't open file: '", path, "'");

        ifs.seekg(0, std::ios_base::end);
        auto size = static_cast<ScmSizeT>(ifs.tellg());
        ifs.seekg(0, std::ios_base::beg);

        auto str = ScmString();
        str.resize(size + 1);

        ifs.read(str.data(), size);
        return std::move(str);
    }

    /**
     * Split string around given delimiters
     * @param str - a string to split
     * @param l - delimiters
     * @param createNullStrs - flag for creating null strings (drops them if false)
     * @return A vector with string_view's
     */
    inline auto split_view(const ScmStrView& str, std::initializer_list<char> l, bool createNullStrs = false)
    -> ScmVector<ScmStrView>
    {
        ScmVector<ScmStrView> vec;
        ScmSizeT start = 0;

        for (ScmSizeT i = 0; i < str.size(); ++i) {
            bool cmp = false;
            for (auto c : l) {
                if (str[i] == c) {
                    cmp = true;
                    break;
                }
            }
            if (cmp) {
                if (createNullStrs || start != i) {
                    vec.emplace_back(str.substr(start, i - start));
                }
                start = i + 1;
            }
        }

        if (start != str.size())
            vec.emplace_back(str.substr(start, str.size()));

        return std::move(vec);
    }
} // namespace scm_utils
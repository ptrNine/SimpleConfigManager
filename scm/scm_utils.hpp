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
    template <typename T, typename ... Types>
    constexpr bool any_of = (std::is_same_v<T, Types> || ...);

    template <typename T>
    constexpr bool floats =
            std::is_floating_point_v<std::remove_const_t<std::remove_reference_t<T>>>;

    template <typename T>
    constexpr bool integers =
            std::is_integral_v<std::remove_const_t<std::remove_reference_t<T>>> && !std::is_same_v<T, bool>;

    template <typename T>
    constexpr bool numbers = integers<T> || floats<T>;

    template <typename T>
    constexpr bool is_string = any_of<T, ScmString, ScmStrView, std::string, std::string_view>;


    // Helper function for combine strings
    template <typename ... ArgT>
    static auto str_join(const ArgT& ... strs) -> std::string {
        return (std::string(strs) + ...);
    }

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

    // File reader exception
    class ScmIfsException : public std::exception {
    public:
        explicit ScmIfsException(std::string error) : _exc(std::move(error)) {}
        const char* what() const noexcept override { return _exc.data(); }

    private:
        std::string _exc;
    };

    class CfgException : public std::exception {
    public:
        explicit CfgException(std::string error) : _exc(std::move(error)) {}
        const char* what() const noexcept override { return _exc.data(); }

    private:
        std::string _exc;
    };

    // Filesystem exception
    class ScmFsException : public std::exception {
    public:
        explicit ScmFsException(std::string error) : _exc(std::move(error)) {}
        const char* what() const noexcept override { return _exc.data(); }

    private:
        std::string _exc;
    };

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

    inline auto split_view(const ScmStrView& str, std::initializer_list<char> l, bool createNullStrs)
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
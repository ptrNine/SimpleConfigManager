#pragma once

#include <iostream>
#include <fstream>

#include "scm_types.hpp"

#ifdef SCM_ASSERTS
    #define SCM_EXCEPTION(EXCEPTION_TYPE, CONDITION, ...) \
        SCM_ASSERTS((CONDITION), "%s", SCM_NAMESPACE::str_join(__VA_ARGS__))
#elif defined SCM_FMT_ASSERTS
    #define SCM_EXCEPTION(EXCEPTION_TYPE, CONDITION, ...) \
        SCM_FMT_ASSERTS((CONDITION), "{}", SCM_NAMESPACE::str_join(__VA_ARGS__))
#else
    #define SCM_EXCEPTION(EXCEPTION_TYPE, CONDITION, ...)          \
        if (!(CONDITION))                                          \
            throw EXCEPTION_TYPE(SCM_NAMESPACE::str_join(__VA_ARGS__))
#endif


#ifndef SCM_NAMESPACE
    #define SCM_NAMESPACE scm
#endif

namespace SCM_NAMESPACE {
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
     * Check if type 'T' is std::string, std::string_view, ScmString, StmStrView or C-style string
     * @tparam T - checked type
     */
    template <typename T>
    constexpr bool is_string = any_of<T, ScmString, ScmStrView, std::string, std::string_view> ||
            (std::is_array_v<T> && std::is_same_v<std::remove_reference_t<decltype(std::declval<T>()[0])>, char>);

    /**
     * Check if type 'T' is C-style array
     * @tparam T - checked type
     */
    template <typename T>
    constexpr bool is_c_array = std::is_array_v<std::remove_const_t<std::remove_reference_t<T>>>;


    /**
     * Remove const and reference from type
     * @tparam T - type
     */
    template <typename T>
    using remove_const_ref = std::remove_const_t<std::remove_reference_t<T>>;


    template <typename T, template <typename...> class Template>
    struct Is_specialization_of : std::false_type {};

    template <template <typename...> class Template, typename... Args>
    struct Is_specialization_of<Template<Args...>, Template> : std::true_type {};


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
    inline auto parent_path(const S& str) -> std::enable_if_t<is_string<S>, S>
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
     * @tparam S1 - first string type (also type of returned string)
     * @tparam S2 - second string type
     * @param str1 - path
     * @param str2 - appended path
     * @return new path
     */
    template <typename S1, typename S2>
    inline auto append_path(const S1& str1, const S2& str2) -> std::enable_if_t<is_string<S1> && is_string<S2>, ScmString>
    {
        auto p1 = ScmString(str1);
        auto p2 = ScmString(str2);

        if (p1.empty() || p2.empty())
            return p1 + p2;
        else {
            if (p1.back() == '/' && p2.front() == '/')
                return p1 + (p2.data() + 1);
            else if (p1.back() == '/' || p2.front() == '/')
                return p1 + p2;
            else
                return p1 + '/' + p2;
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
} // namespace SCM_NAMESPACE

namespace scm_details {
    using StrViewCref = const ScmStrView&;

    auto unpack(const ScmStrView& name, const ScmStrView& section, const ScmStrView& str, std::size_t required) -> ScmVector<ScmStrView>;

    template <typename T>
    auto superCast(StrViewCref str, StrViewCref, StrViewCref)
    -> std::enable_if_t<SCM_NAMESPACE::numbers<T>, T>;

    template <typename T>
    auto superCast(StrViewCref str, StrViewCref, StrViewCref)
    -> std::enable_if_t<SCM_NAMESPACE::any_of<T, ScmString, ScmStrView, std::string_view, std::string>, T>;

    template <typename T>
    auto superCast(StrViewCref str, StrViewCref name, StrViewCref section)
    -> std::enable_if_t<std::is_same_v<T, bool>, bool>;

    template <typename A, typename T = std::remove_reference_t<decltype(std::declval<A>()[0])>, ScmSizeT _Size = sizeof(A)/sizeof(T)>
    auto superCast(StrViewCref str, StrViewCref name, StrViewCref section)
    -> std::enable_if_t<SCM_NAMESPACE::any_of<A, ScmArray<T, _Size>, std::array<T, _Size>>, A>;

    template <typename A, typename T = std::remove_reference_t<decltype(std::declval<A>()[0])>>
    auto superCast(StrViewCref str, StrViewCref name, StrViewCref section)
    -> std::enable_if_t<SCM_NAMESPACE::any_of<A, ScmVector<T>, std::vector<T>>, A>;

    template <typename A, typename T, ScmSizeT _Size>
    auto superCast(StrViewCref str, StrViewCref name, StrViewCref section)
    -> std::enable_if_t<SCM_NAMESPACE::any_of<A, ScmArray<T, _Size>, std::array<T, _Size>>, A>;

    template <typename A, typename T>
    auto superCast(StrViewCref str, StrViewCref name, StrViewCref section)
    -> std::enable_if_t<SCM_NAMESPACE::any_of<A, ScmVector<T>, std::vector<T>>, A>;

    template <typename T>
    auto superCast(StrViewCref str, StrViewCref name, StrViewCref section)
    -> std::enable_if_t<SCM_NAMESPACE::Is_specialization_of<T, std::tuple>::value, T>;

    template <typename T>
    auto superCast(StrViewCref str, StrViewCref name, StrViewCref section)
    -> std::enable_if_t<
            SCM_NAMESPACE::Is_specialization_of<T, std::pair>::value ||
            SCM_NAMESPACE::Is_specialization_of<T, ScmPair>::value, T>;
}

#define SCM_SUPERCAST() \
superCast(const ScmStrView& str, const ScmStrView& name, const ScmStrView& section)

#define SCM_SUPERCAST_ARGS name, section

/**
 * Unpack list of values from string
 * @param REQUIRED - numbers of required values (0 if any number)
 */
#define SCM_UNPACK(REQUIRED) unpack(name, section, str, REQUIRED)
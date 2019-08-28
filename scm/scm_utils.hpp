#pragma once

#include <iostream>
#include <fstream>

#include "scm_types.hpp"

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

    template <typename S>
    inline auto append_path(const S& p1, const S& p2) -> std::enable_if_t<is_string<S>, ScmString>
    {
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

    template <typename S>
    inline auto read_file_to_string(const S& name) -> std::enable_if_t<is_string<S>, ScmString>
    {
        auto path = ScmString(name);
        auto ifs = std::ifstream(path.data(), std::ios_base::binary | std::ios_base::in);

//        if (!_ifs.is_open())
//            RABORTF("Can't open file: \'{}\'", path);

        ifs.seekg(0, std::ios_base::end);
        auto size = static_cast<ScmSizeT>(ifs.tellg());
        ifs.seekg(0, std::ios_base::beg);

        auto str = ScmString();
        str.resize(size + 1);

        ifs.read(str.data(), size);
        return std::move(str);
    }

    template <typename S = ScmString>
    inline auto split_view(const ScmString& str, std::initializer_list<char> l, bool createNullStrs)
    -> std::enable_if_t<is_string<S>,
    ScmVector<ScmStrView>>
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
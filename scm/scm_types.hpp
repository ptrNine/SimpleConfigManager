#pragma once

using ScmSizeT = std::size_t;
using ScmChar8 = char;


#ifdef SCM_TYPE_STRING
    using ScmString = SCM_TYPE_STRING;
#else
    #include <string>
    using ScmString = std::string;
#endif


#ifdef SCM_TYPE_STRING_VIEW
    using ScmStrView = SCM_TYPE_STRING_VIEW;
#else
    #include <string_view>
    using ScmStrView = std::string_view;
#endif


#ifdef SCM_TYPE_PAIR
    template <typename F, typename S>
    using ScmPair = SCM_TYPE_PAIR<F, S>;
#else
    #include <utility>
    template <typename F, typename S>
    using ScmPair = std::pair<F, S>;
#endif


#ifdef SCM_TYPE_VECTOR
    template <typename T>
    using ScmVector = SCM_TYPE_VECTOR<T>;
#else
    #include <vector>
    template <typename T>
    using ScmVector = std::vector<T>;
#endif


#ifdef SCM_TYPE_ARRAY
    template <typename T, ScmSizeT _Sz>
    using ScmArray = SCM_TYPE_ARRAY<T, _Sz>;
#else
    #include <array>
    template <typename T, ScmSizeT _Sz>
    using ScmArray = std::array<T, _Sz>;
#endif


#ifdef SCM_TYPE_MAP
    template <typename K, typename V>
    using ScmMap = SCM_TYPE_MAP<K, V>;
#else
    #include <unordered_map>
    template <typename K, typename V>
    using ScmMap = std::unordered_map<K, V>;
#endif

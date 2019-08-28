#pragma once

#include <string>
#include <vector>
#include <unordered_map>

using ScmString  = std::string;
using ScmStrView = std::string_view;
using ScmSizeT   = std::size_t;
using ScmChar8   = char;

template <typename F, typename S>
using ScmPair = std::pair<F, S>;

template <typename T>
using ScmVector = std::vector<T>;

template <typename T, ScmSizeT _Sz>
using ScmArray = std::array<T, _Sz>;

template <typename K, typename V>
using ScmMap = std::unordered_map<K, V>;

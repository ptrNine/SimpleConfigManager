#pragma once

#include <type_traits>
#include "scm_utils.hpp"

#define white_space(c) ((c) == ' ' || (c) == '\t')
#define valid_digit(c) ((c) >= '0' && (c) <= '9')

#ifndef SCM_NAMESPACE
    #define SCM_NAMESPACE scm
#endif

namespace SCM_NAMESPACE {
    template<typename T>
    auto aton(const ScmStrView& str, const ScmStrView& name, const ScmStrView& section)
    -> std::enable_if_t<std::is_floating_point_v<T>, T>
    {
        auto p = str.cbegin();

        while (white_space(*p) && p != str.end()) { ++p; }

        auto r = static_cast<T>(0);

        // Get sign
        bool neg = false;
        if (*p == '-' && p != str.end()) {
            neg = true;
            ++p;
        }
        else if (*p == '+' && p != str.end()) {
            neg = false;
            ++p;
        }

        // Get the digits before decimal point
        while (valid_digit(*p) && p != str.end()) {
            r = (r * 10) + (*p - '0');
            ++p;
        }

        SCM_EXCEPTION(ScmAtonException, p != str.cbegin(),
                      "Invalid digit '", std::string(1, *p), "' in float '",
                      name, "' in section [", section, "]");

        // Get the digits after decimal point
        if (*p == '.' && p != str.end()) {
            T f = 0.0;
            std::size_t scale = 1;
            ++p;
            while (valid_digit(*p) && p != str.end()) {
                f = (f * 10) + (*p - '0');
                ++p;
                scale *= 10;
            }
            r += f / static_cast<T>(scale);
        }

        // Get the digits after the "e"/"E" (exponenet)
        if ((*p == 'e' || *p == 'E') && p != str.end()) {
            std::ptrdiff_t e = 0;

            bool negE = false;
            ++p;

            if (p != str.end()) {
                if      (*p == '-') { negE = true;  ++p; }
                else if (*p == '+') { negE = false; ++p; }
            }

            auto st = p;

            while (valid_digit(*p) && p != str.end()) { e = (e * 10) + (*p - '0'); ++p; }

            SCM_EXCEPTION(ScmAtonException, p != st,
                          "Missing number after exponent in float '",
                          name, "' in section [", section, "]");

            if (!neg && e > std::numeric_limits<T>::max_exponent10) {
                e = std::numeric_limits<T>::max_exponent10;
            }
            else if (e < std::numeric_limits<T>::min_exponent10) {
                e = -std::numeric_limits<T>::min_exponent10;
            }

            auto scaleE = static_cast<T>(1.0);
            while (e >= 50) { scaleE *= static_cast<T>(1E50); e -= 50; }
            while (e > 0) { scaleE *= 10; --e; }

            if (negE) {
                r /= scaleE;
            }
            else {
                r *= scaleE;
            }
        }

        SCM_EXCEPTION(ScmAtonException, p == str.end() || valid_digit(*p),
                      "Invalid digit '", std::string(1, *p), "' in float '",
                      name, "' in section [", section, "]");

        if (neg) { r = -r; }
        return r;
    }

    template<typename T>
    auto aton(const ScmStrView& str, const ScmStrView& name, const ScmStrView& section)
    -> std::enable_if_t<std::is_signed_v<T> && std::is_integral_v<T>, T>
    {
        auto p = str.cbegin();

        while (white_space(*p) && p != str.end()) { ++p; }

        auto r = static_cast<T>(0);

        bool neg = false;
        if (p != str.end()) {
            if      (*p == '-') { neg = true;  ++p; }
            else if (*p == '+') { neg = false; ++p; }
        }

        while (valid_digit(*p) && p != str.end()) {
            r = (r * 10) + (*p - '0');
            ++p;
        }

        SCM_EXCEPTION(ScmAtonException, p == str.end() || valid_digit(*p),
                      "Invalid digit '", std::string(1, *p), "' in integer '",
                      name, "' in section [", section, "]");

        if (neg) { r = -r; }
        return r;
    }

    template<typename T>
    auto aton(const ScmStrView& str, const ScmStrView& name, const ScmStrView& section)
    -> std::enable_if_t<std::is_unsigned_v<T>, T>
    {
        auto p = str.cbegin();

        while (white_space(*p) && p != str.end()) { ++p; }

        auto r = static_cast<T>(0);

        if (*p == '+' && p != str.end()) { ++p; }

        while (valid_digit(*p) && p != str.end()) {
            r = (r * 10) + (*p - '0');
            ++p;
        }

        SCM_EXCEPTION(ScmAtonException, p == str.end() || valid_digit(*p),
                      "Invalid digit '", std::string(1, *p), "' in unsigned integer '",
                      name, "' in section [", section, "]");

        return r;
    }
} // namespace SCM_NAMESPACE

#undef white_space
#undef valid_digit

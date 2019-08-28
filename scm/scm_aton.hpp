#pragma once

#include <type_traits>

#define white_space(c) ((c) == ' ' || (c) == '\t')
#define valid_digit(c) ((c) >= '0' && (c) <= '9')

namespace scm_aton {
    template<typename T>
    auto aton(const char *p) -> std::enable_if_t<std::is_floating_point_v<T>, T> {

        while (white_space(*p)) { ++p; }

        auto r = static_cast<T>(0);

        //get sign
        bool neg = false;
        if (*p == '-') {
            neg = true;
            ++p;
        }
        else if (*p == '+') {
            neg = false;
            ++p;
        }

        //get the digits before decimal point
        while (valid_digit(*p)) {
            r = (r * 10) + (*p - '0');
            ++p;
        }

        //get the digits after decimal point
        if (*p == '.') {
            T f = 0.0;
            std::size_t scale = 1;
            ++p;
            while (valid_digit(*p)) {
                f = (f * 10) + (*p - '0');
                ++p;
                scale *= 10;
            }
            r += f / static_cast<T>(scale);
        }

        //R_ASSERTF(!read_check, "string '%s' not a number", p);

        // Get the digits after the "e"/"E" (exponenet)
        if (*p == 'e' || *p == 'E') {
            std::ptrdiff_t e = 0;

            bool negE = false;
            ++p;
            if (*p == '-') { negE = true; ++p; }
            else if (*p == '+') { negE = false; ++p; }

            while (valid_digit(*p)) { e = (e * 10) + (*p - '0'); ++p; }
            //R_ASSERTF(!read_check, "string '%s' not a number", p);

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

        if (neg) { r = -r; }
        return r;
    }

    template<typename T>
    auto aton(const char *p) -> std::enable_if_t<std::is_signed_v<T> && std::is_integral_v<T>, T> {
        while (white_space(*p)) { ++p; }

        auto r = static_cast<T>(0);

        bool neg = false;
        if (*p == '-') {
            neg = true;
            ++p;
        }
        else if (*p == '+') {
            neg = false;
            ++p;
        }

        while (valid_digit(*p)) {
            r = (r * 10) + (*p - '0');
            ++p;
        }

        if (neg) { r = -r; }
        return r;
    }

    template<typename T>
    auto aton(const char *p) -> std::enable_if_t<std::is_unsigned_v<T>, T> {
        while (white_space(*p)) { ++p; }

        auto r = static_cast<T>(0);

        if (*p == '+') { ++p; }

        while (valid_digit(*p)) {
            r = (r * 10) + (*p - '0');
            ++p;
        }

        return r;
    }

    template <typename T>
    inline auto aton(const char* p, T& value) {
        value = aton<T>(p);
    }
}

#undef white_space
#undef valid_digit

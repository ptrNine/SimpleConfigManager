#pragma once

#include "scm_details.hpp"
#include "scm_aton.hpp"

#define IA inline auto
#define IS_WHITE_SPACE(CH) ((CH) == ' ' || (CH) == '\t')

#ifndef SCM_NAMESPACE
    #define SCM_NAMESPACE scm
#endif

namespace SCM_NAMESPACE {
    /**
     * Parse config file and all includes
     * @param filepath - path to file
     */
    IA parse(const ScmStrView& filepath) {
        scm_details::parse(filepath);
    }

    /**
     * Clear config data
     */
    IA clear() {
        scm_details::cfg_data().clear();
    }

    /**
     * Clear data and parse config file
     */
    IA reload(const ScmStrView& filepath) {
        clear();
        parse(filepath);
    }

    /**
     * Read one value from cfg
     * @tparam T - value type
     * @param key - value key
     * @param section - section name. If unused - perform read from global namespace
     * @return Value with T type
     */
    template <typename T>
    IA read(const ScmStrView& key, const ScmStrView& section = scm_details::GLOBAL_NAMESPACE) {
        using namespace scm_details;

        return superCast<T>(cfg_data().getValue(String(section), String(key)), key, section);
    }

    /**
     * Read two ore more values from cfg
     * @tparam T1 - type of first value
     * @tparam T2 - type of second value
     * @tparam Ts - type of other values
     * @param key - value key
     * @param section - section name. If unused - perform read from global namespace
     * @return Tuple of read values
     */
    template <typename T1, typename T2, typename... Ts>
    IA read(const ScmStrView& key, const ScmStrView& section = scm_details::GLOBAL_NAMESPACE) {
        using namespace scm_details;

        auto str = cfg_data().getValue(String(section), String(key));
        auto vec = unpack(key, section, str, sizeof...(Ts) + 2);
        auto is  = std::make_index_sequence<sizeof...(Ts) + 2>();

        return readTupleImpl<T1, T2, Ts...>(vec, key, section, is);
    }

    /**
     * Read one value from cfg if key or section exists
     * @tparam T - value type
     * @param key - value key
     * @param section - section name
     * @param default_val - default value
     * @return Value with T type or default_val if key or section doesn't exists
     */
    template <typename T>
    IA read_ie(const ScmStrView& key, const ScmStrView& section, const T& default_val) {
        using namespace scm_details;

        auto str = cfg_data().valueOpt(String(section), String(key));

        if (str)
            return superCast<T>(*str, key, section);
        else
            return default_val;
    }

    /**
     * Read two ore more values from cfg if key or section exists
     * @tparam T1 - type of first value
     * @tparam T2 - type of second value
     * @tparam Ts - type of other values
     * @param key - value key
     * @param section - section name
     * @param def_val1 - first default value
     * @param def_val2 - second default value
     * @param default_vals - other default values
     * @return Tuple of read values or default_vals if key or section doesn't exists
     */
    template <typename T1, typename T2, typename... Ts>
    IA read_ie(const ScmStrView& key, const ScmStrView& section,
            const T1& def_val1, const T2& def_val2, const Ts& ... default_vals)
    {
        using namespace scm_details;

        auto str = cfg_data().valueOpt(String(section), String(key));

        if (str) {
            auto vec = unpack(key, section, *str, sizeof...(Ts) + 2);
            auto is = std::make_index_sequence<sizeof...(Ts) + 2>();

            return readTupleImpl<T1, T2, Ts...>(vec, key, section, is);
        }
        else {
            return std::make_tuple(default_vals...);
        }
    }

    /**
     * Read one value from cfg from global namespace if key or section exists
     * @tparam T - value type
     * @tparam Ts - other values types
     * @param key - value key
     * @param default_val - default value
     * @param default_vals - other default values
     * @return Value with T type (tuple with <T, Ts...>) or default_val (tuple of default_vals) if key or section doesn't exists
     */
    template <typename T, typename... Ts>
    IA read_ie(const ScmStrView& key, const T& default_val, const Ts& ... default_vals) {
        using namespace scm_details;

        return read_ie<T, Ts...>(key, GLOBAL_NAMESPACE, default_val, default_vals...);
    }

    /**
     * Read one value from cfg if key exists
     * @tparam T - value type
     * @param key - value key
     * @param section - section name
     * @param default_val - default value
     * @return Value with T type or default_val if key doesn't exists
     */
    template <typename T>
    IA read_ike(const ScmStrView& key, const ScmStrView& section, const T& default_val) {
        using namespace scm_details;

        auto sect = cfg_data().getSection(String(section));
        auto str  = sect.valueOpt(String(key));

        if (str)
            return superCast<T>(*str, key, section);
        else
            return default_val;
    }

    /**
     * Read two ore more values from cfg if key exists
     * @tparam T1 - type of first value
     * @tparam T2 - type of second value
     * @tparam Ts - type of other values
     * @param key - value key
     * @param section - section name
     * @param def_val1 - first default value
     * @param def_val2 - second default value
     * @param default_vals - other default values
     * @return Tuple of read values or default_vals if key doesn't exists
     */
    template <typename T1, typename T2, typename... Ts>
    IA read_ike(const ScmStrView& key, const ScmStrView& section,
                const T1& def_val1, const T2& def_val2, const Ts& ... default_vals)
    {
        using namespace scm_details;

        auto sect = cfg_data().getSection(String(section));
        auto str  = sect.valueOpt(String(key));

        if (str) {
            auto vec = unpack(key, section, *str, sizeof...(Ts) + 2);
            auto is = std::make_index_sequence<sizeof...(Ts) + 2>();

            return readTupleImpl<T1, T2, Ts...>(vec, key, section, is);
        }
        else {
            return std::make_tuple(default_vals...);
        }
    }

    /**
     * Read one value from cfg from global namespace if key exists
     * @tparam T - value type
     * @tparam Ts - other values types
     * @param key - value key
     * @param default_val - default value
     * @param default_vals - other default values
     * @return Value with T type (tuple with <T, Ts...>) or default_val (tuple of default_vals) if key doesn't exists
     */
    template <typename T, typename... Ts>
    IA read_ike(const ScmStrView& key, const T& default_val, const Ts& ... default_vals) {
        using namespace scm_details;

        return read_ike<T, Ts...>(key, GLOBAL_NAMESPACE, default_val, default_vals...);
    }
} // namespace SCM_NAMESPACE


#undef IA // inline auto
#undef IS_WHITE_SPACE
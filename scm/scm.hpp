#pragma once

#include "scm_config.hpp"
#include "scm_details.hpp"
#include "scm_aton.hpp"

#define IA inline auto
#define SIA static inline auto
#define IS_WHITE_SPACE(CH) ((CH) == ' ' || (CH) == '\t')

#ifdef SCM_NAMESPACE
namespace SCM_NAMESPACE {
#endif // SCM_NAMESPACE

    class ConfigManager {
    public:
        using SizeT       = ScmSizeT;

        using String      = ScmString;
        using StringCref  = const String&;
        using StringRef   = String&;
        using StringRval  = String&&;

        using StrView     = ScmStrView;
        using StrViewCref = const StrView&;
        using StrViewRef  = StrView&;
        using StrViewRval = StrView&&;

        using StrvStrvMap = ScmMap<StrView, StrView>;
        using StrvVector  = ScmVector<StrView>;
        using StrViewPair = ScmPair<StrView, StrView>;

        using CfgException = scm_details::CfgException;

    public:
        // Read if section or value exist, else return default value
        template <typename T>
        static IA force_read_ie(StrViewCref name, const T& def_val) {
            return force_read_ie<T>(name, scm_details::GLOBAL_NAMESPACE, def_val);
        }
        template <typename T1, typename T2, typename... Ts>
        static IA force_read_ie(StrViewCref name, const std::tuple<T1, T2, Ts...>& def_vals) {
            return force_read_ie<T1, T2, Ts...>(name, scm_details::GLOBAL_NAMESPACE, def_vals);
        }
        template <typename T>
        static T force_read_ie(StrViewCref name, StrViewCref section, const T& def_val) {
            auto str = scm_details::cfgData().valueOpt(String(section), String(name));
            if (str)
                return scm_details::superCast<T>(*str, name, section);
            else
                return def_val;
        }
        template <typename T1, typename T2, typename... Ts>
        static auto force_read_ie(StrViewCref name, StrViewCref section, const std::tuple<T1, T2, Ts...>& def_vals) {
            auto str = scm_details::cfgData().valueOpt(String(section), String(name));
            if (str)
                return scm_details::superCast<T1, T2, Ts...>(*str, name, section);
            else
                return def_vals;
        }

        // Read if value exist. Return default value if not, assert if section doesn't exists
        template <typename T>
        static IA force_read_ive(StrViewCref name, const T& def_val) {
            return force_read_ive<T>(name, scm_details::GLOBAL_NAMESPACE, def_val);
        }
        template <typename T1, typename T2, typename... Ts>
        static IA force_read_ive(StrViewCref name, const std::tuple<T1, T2, Ts...>& def_vals) {
            return force_read_ive<T1, T2, Ts...>(name, scm_details::GLOBAL_NAMESPACE, def_vals);
        }
        template <typename T>
        static T force_read_ive(StrViewCref name, StrViewCref section, const T& def_val) {
            auto sect = scm_details::cfgData().getSection(String(section));
            auto str  = sect.valueOpt(String(name));
            if (str)
                return scm_details::superCast<T>(*str, name, section);
            else
                return def_val;
        }
        template <typename T1, typename T2, typename... Ts>
        static auto force_read_ive(StrViewCref name, StrViewCref section, const std::tuple<T1, T2, Ts...>& def_vals) {
            auto sect = scm_details::cfgData().getSection(String(section));
            auto str  = sect.valueOpt(String(name));
            if (str)
                return scm_details::superCast<T1, T2, Ts...>(*str, name, section);
            else
                return def_vals;
        }

        template <typename T>
        static T force_read(StrViewCref name, StrViewCref section = scm_details::GLOBAL_NAMESPACE) {
            return scm_details::superCast<T>(readString(name, section), name, section);
        }

        template <typename T1, typename T2, typename... Ts>
        static auto force_read(StrViewCref name, StrViewCref section = scm_details::GLOBAL_NAMESPACE) {
            auto str = readString(name, section);
            auto vec = scm_details::unpack(name, section, str, sizeof...(Ts) + 2);
            auto is  = std::make_index_sequence<sizeof...(Ts) + 2>();

            return scm_details::readTupleImpl<T1, T2, Ts...>(vec, name, section, is);
        }


    private:

        // Basic read method
        SIA readString(StrViewCref name, StrViewCref section) -> StringRef {
            return scm_details::cfgData().getValue(String(section), String(name));
        }








        void load() {
            scm_details::processFileTask();
            scm_details::cfgData().reloadParents();
        }


        // Singleton impl
    public:
        ConfigManager(const ConfigManager&) = delete;
        ConfigManager& operator= (const ConfigManager&) = delete;

        static ConfigManager& instance() {
            static ConfigManager inst;
            return inst;
        }

    private:
        ConfigManager () {
            scm_details::cfg_state()._onCreate  = true;
            load();
            scm_details::cfg_state()._onCreate  = false;
            scm_details::cfg_state()._isCreated = true;
        }
        ~ConfigManager() = default;
    };


    using StrViewCref = const ScmStrView&;

    /**
     * Add entry file to loading
     * @param path - path to config entry file
     */
    inline void addCfgEntry(const ScmString& path) {
        scm_details::cfg_state().addCfgEntry(path);
    }

    /**
     * Remove all entry files
     */
    inline void resetCfgEntries() {
        scm_details::cfg_state().clearCfgEntries();
    }

    /**
     * Get instance of ConfigManager
     * @return reference to ConfigManager
     */
    inline ConfigManager& cfg() { return ConfigManager::instance(); }

    /**
     * Basic config read
     * @tparam Ts - type (types) of value
     * @param name - value name
     * @param section - section name. If unused - perform read from global namespace
     * @return Value with T type, or tuple if more one types passed
     */
    template<typename... Ts>
    IA read(StrViewCref name, StrViewCref section = scm_details::GLOBAL_NAMESPACE) {
        cfg();
        return ConfigManager::force_read<Ts...>(name, section);
    }

    ///////////// READ IF SECTION OR VALUE EXISTS //////////////

    /**
     * Basic config read from global namespace if value or section exists
     * @tparam T - type of value
     * @param name - value name
     * @param def_val - default value with T type
     * @return Value with T type or default val
     */
    template <typename T>
    IA read_ie(StrViewCref name, const T& def_val) {
        cfg();
        return ConfigManager::force_read_ie<T>(name, def_val);
    }
    /**
     * Basic config read from global namespace if value or section exists
     * @tparam T1, T2, Ts - types of value
     * @param name - value name
     * @param def_vals - default value tuple with T1, T2, Ts types
     * @return Tuple of T1, T2, Ts or default tuple
     */
    template <typename T1, typename T2, typename... Ts>
    IA read_ie(StrViewCref name, const std::tuple<T1, T2, Ts...>& def_vals) {
        cfg();
        return ConfigManager::force_read_ie<T1, T2, Ts...>(name, def_vals);
    }
    /**
     * Basic config read if value or section exists
     * @tparam T - type of value
     * @param name - value name
     * @param section - section name
     * @param def_val - default value with T type
     * @return Value with T type or default val
     */
    template <typename T>
    IA read_ie(StrViewCref name, StrViewCref section, const T& def_val) {
        cfg();
        return ConfigManager::force_read_ie<T>(name, section, def_val);
    }
    /**
     * Basic config read if value or section exists
     * @tparam T1, T2, Ts - types of value
     * @param name - value name
     * @param section - section name
     * @param def_vals - default value tuple with T1, T2, Ts types
     * @return Tuple of T1, T2, Ts or default tuple
     */
    template <typename T1, typename T2, typename... Ts>
    IA read_ie(StrViewCref name, StrViewCref section, const std::tuple<T1, T2, Ts...>& def_vals) {
        cfg();
        return ConfigManager::force_read_ie<T1, T2, Ts...>(name, section, def_vals);
    }

    ///////////// READ IF VALUE EXISTS //////////////

    /**
     * Basic config read from global namespace if value exists (Assert if section does't exist)
     * @tparam T - type of value
     * @param name - value name
     * @param def_val - default value with T type
     * @return Value with T type or default val
     */
    template <typename T>
    IA read_ive(StrViewCref name, const T& def_val) {
        cfg();
        return ConfigManager::force_read_ive<T>(name, def_val);
    }
    /**
     * Basic config read from global namespace if value exists (Assert if section does't exist)
     * @tparam T1, T2, Ts - types of value
     * @param name - value name
     * @param def_vals - default value tuple with T1, T2, Ts types
     * @return Tuple of T1, T2, Ts or default tuple
     */
    template <typename T1, typename T2, typename... Ts>
    IA read_ive(StrViewCref name, const std::tuple<T1, T2, Ts...>& def_vals) {
        cfg();
        return ConfigManager::force_read_ive<T1, T2, Ts...>(name, def_vals);
    }
    /**
     * Basic config read if value exists (Assert if section does't exist)
     * @tparam T - type of value
     * @param name - value name
     * @param section - section name
     * @param def_val - default value with T type
     * @return Value with T type or default val
     */
    template <typename T>
    IA read_ive(StrViewCref name, StrViewCref section, const T& def_val) {
        cfg();
        return ConfigManager::force_read_ive<T>(name, section, def_val);
    }
    /**
     * Basic config read if value exists (Assert if section does't exist)
     * @tparam T1, T2, Ts - types of value
     * @param name - value name
     * @param section - section name
     * @param def_vals - default value tuple with T1, T2, Ts types
     * @return Tuple of T1, T2, Ts or default tuple
     */
    template <typename T1, typename T2, typename... Ts>
    IA read_ive(StrViewCref name, StrViewCref section, const std::tuple<T1, T2, Ts...>& def_vals) {
        cfg();
        return ConfigManager::force_read_ive<T1, T2, Ts...>(name, section, def_vals);
    }



    ////////////// FORCE READ FUNCTIONS ///////////////////
    ///////////// READ IF SECTION OR VALUE EXISTS //////////////

    /**
     * Read from global namespace if value or section exists
     * Read cfg value without full loading (use for prevent recursive ConfigManager initialization)
     * @tparam T - type of value
     * @param name - value name
     * @param def_val - default value with T type
     * @return Value with T type or default val
     */
    template <typename T>
    IA force_read_ie(StrViewCref name, const T& def_val) {
        if (scm_details::cfg_state().onCreate())
            return ConfigManager::force_read_ie<T>(name, def_val);
        else
            return read_ie<T>(name, def_val);
    }
    /**
     * Read from global namespace if value or section exists
     * Read cfg value without full loading (use for prevent recursive ConfigManager initialization)
     * @tparam T1, T2, Ts - types of value
     * @param name - value name
     * @param def_vals - default value tuple with T1, T2, Ts types
     * @return Tuple of T1, T2, Ts or default tuple
     */
    template <typename T1, typename T2, typename... Ts>
    IA force_read_ie(StrViewCref name, const std::tuple<T1, T2, Ts...>& def_vals) {
        if (scm_details::cfg_state().onCreate())
            return ConfigManager::force_read_ie<T1, T2, Ts...>(name, def_vals);
        else
            return read_ie<T1, T2, Ts...>(name, def_vals);
    }
    /**
     * Read if value or section exists
     * Read cfg value without full loading (use for prevent recursive ConfigManager initialization)
     * @tparam T - type of value
     * @param name - value name
     * @param section - section name
     * @param def_val - default value with T type
     * @return Value with T type or default val
     */
    template <typename T>
    IA force_read_ie(StrViewCref name, StrViewCref section, const T& def_val) {
        if (scm_details::cfg_state().onCreate())
            return ConfigManager::force_read_ie<T>(name, section, def_val);
        else
            return read_ie<T>(name, section, def_val);
    }
    /**
     * Read if value or section exists
     * Read cfg value without full loading (use for prevent recursive ConfigManager initialization)
     * @tparam T1, T2, Ts - types of value
     * @param name - value name
     * @param section - section name
     * @param def_vals - default value tuple with T1, T2, Ts types
     * @return Tuple of T1, T2, Ts or default tuple
     */
    template <typename T1, typename T2, typename... Ts>
    IA force_read_ie(StrViewCref name, StrViewCref section, const std::tuple<T1, T2, Ts...>& def_vals) {
        if (scm_details::cfg_state().onCreate())
            return ConfigManager::force_read_ie<T1, T2, Ts...>(name, section, def_vals);
        else
            return read_ie<T1, T2, Ts...>(name, section, def_vals);
    }

    ///////////// READ IF VALUE EXISTS //////////////

    /**
     * Read from global namespace if value exists (Assert if section does't exist)
     * Read cfg value without full loading (use for prevent recursive ConfigManager initialization)
     * @tparam T - type of value
     * @param name - value name
     * @param def_val - default value with T type
     * @return Value with T type or default val
     */
    template <typename T>
    IA force_read_ive(StrViewCref name, const T& def_val) {
        if (scm_details::cfg_state().onCreate())
            return ConfigManager::force_read_ive<T>(name, def_val);
        else
            return read_ive<T>(name, def_val);
    }
    /**
     * Read from global namespace if value exists (Assert if section does't exist)
     * Read cfg value without full loading (use for prevent recursive ConfigManager initialization)
     * @tparam T1, T2, Ts - types of value
     * @param name - value name
     * @param def_vals - default value tuple with T1, T2, Ts types
     * @return Tuple of T1, T2, Ts or default tuple
     */
    template <typename T1, typename T2, typename... Ts>
    IA force_read_ive(StrViewCref name, const std::tuple<T1, T2, Ts...>& def_vals) {
        if (scm_details::cfg_state().onCreate())
            return ConfigManager::force_read_ive<T1, T2, Ts...>(name, def_vals);
        else
            return read_ive<T1, T2, Ts...>(name, def_vals);
    }
    /**
     * Read if value exists (Assert if section does't exist)
     * Read cfg value without full loading (use for prevent recursive ConfigManager initialization)
     * @tparam T - type of value
     * @param name - value name
     * @param section - section name
     * @param def_val - default value with T type
     * @return Value with T type or default val
     */
    template <typename T>
    IA force_read_ive(StrViewCref name, StrViewCref section, const T& def_val) {
        if (scm_details::cfg_state().onCreate())
            return ConfigManager::force_read_ive<T>(name, section, def_val);
        else
            return read_ive<T>(name, section, def_val);
    }
    /**
     * Read if value exists (Assert if section does't exist)
     * Read cfg value without full loading (use for prevent recursive ConfigManager initialization)
     * @tparam T1, T2, Ts - types of value
     * @param name - value name
     * @param section - section name
     * @param def_vals - default value tuple with T1, T2, Ts types
     * @return Tuple of T1, T2, Ts or default tuple
     */
    template <typename T1, typename T2, typename... Ts>
    IA force_read_ive(StrViewCref name, StrViewCref section, const std::tuple<T1, T2, Ts...>& def_vals) {
        if (scm_details::cfg_state().onCreate())
            return ConfigManager::force_read_ive<T1, T2, Ts...>(name, section, def_vals);
        else
            return read_ive<T1, T2, Ts...>(name, section, def_vals);
    }

    /**
    * Read cfg value without full loading (use for prevent recursive ConfigManager initialization)
    * @tparam Ts - type (types) of value
    * @param name - value name
    * @param section - section name. If unused - perform read from global namespace
    * @return Value with T type, or tuple if more one types passed
    */
    template<typename... Ts>
    IA force_read(
            const std::string_view &name,
            const std::string_view &section = scm_details::GLOBAL_NAMESPACE) {
        if (scm_details::cfg_state().onCreate())
            return ConfigManager::force_read<Ts...>(name, section);
        else
            return read<Ts...>(name, section);
    }

#ifdef SCM_NAMESPACE
} // namespace SCM_NAMESPACE
#endif // SCM_NAMESPACE


#undef IA // inline auto
#undef SIA // static inline auto
#undef IS_WHITE_SPACE
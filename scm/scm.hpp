#pragma once

#include "scm_config.hpp"
#include "scm_details.hpp"
#include "scm_aton.hpp"

#define IA inline auto
#define SIA static inline auto
#define IS_WHITE_SPACE(CH) ((CH) == ' ' || (CH) == '\t')


////////////////////////////// Config manager //////////////////////////////////

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

        using CfgException = cfg_detls::CfgException;

    public:
        template <typename... Ts>
        IA read(StrViewCref name, StrViewCref section = cfg_detls::GLOBAL_NAMESPACE) {
            return force_read<Ts...>(name, section);
        }

        // Read if section or value exist, else return default value
        template <typename T>
        IA read_ie(StrViewCref name, const T& def_val) {
            return force_read_ie<T>(name, def_val);
        }
        template <typename T1, typename T2, typename... Ts>
        IA read_ie(StrViewCref name, const std::tuple<T1, T2, Ts...>& def_vals) {
            return force_read_ie<T1, T2, Ts...>(name, def_vals);
        }
        template <typename T>
        IA read_ie(StrViewCref name, StrViewCref section, const T& def_val) {
            return force_read_ie<T>(name, section, def_val);
        }
        template <typename T1, typename T2, typename... Ts>
        IA read_ie(StrViewCref name, StrViewCref section, const std::tuple<T1, T2, Ts...>& def_vals) {
            return force_read_ie<T1, T2, Ts...>(name, section, def_vals);
        }

        // Read if value exist. Return default value if not, assert if section doesn't exists
        template <typename T>
        IA read_ive(StrViewCref name, const T& def_val) {
            return force_read_ive<T>(name, def_val);
        }
        template <typename T1, typename T2, typename... Ts>
        IA read_ive(StrViewCref name, const std::tuple<T1, T2, Ts...>& def_vals) {
            return force_read_ive<T1, T2, Ts...>(name, def_vals);
        }
        template <typename T>
        IA read_ive(StrViewCref name, StrViewCref section, const T& def_val) {
            return force_read_ive<T>(name, section, def_val);
        }
        template <typename T1, typename T2, typename... Ts>
        IA read_ive(StrViewCref name, StrViewCref section, const std::tuple<T1, T2, Ts...>& def_vals) {
            return force_read_ive<T1, T2, Ts...>(name, section, def_vals);
        }


        // Static methods

        // Read if section or value exist, else return default value
        template <typename T>
        static IA force_read_ie(StrViewCref name, const T& def_val) {
            return force_read_ie<T>(name, cfg_detls::GLOBAL_NAMESPACE, def_val);
        }
        template <typename T1, typename T2, typename... Ts>
        static IA force_read_ie(StrViewCref name, const std::tuple<T1, T2, Ts...>& def_vals) {
            return force_read_ie<T1, T2, Ts...>(name, cfg_detls::GLOBAL_NAMESPACE, def_vals);
        }
        template <typename T>
        static T force_read_ie(StrViewCref name, StrViewCref section, const T& def_val) {
            auto str = cfg_detls::cfgData().valueOpt(String(section), String(name));
            if (str)
                return superCast<T>(*str, name, section);
            else
                return def_val;
        }
        template <typename T1, typename T2, typename... Ts>
        static auto force_read_ie(StrViewCref name, StrViewCref section, const std::tuple<T1, T2, Ts...>& def_vals) {
            auto str = cfg_detls::cfgData().valueOpt(String(section), String(name));
            if (str)
                return superCast<T1, T2, Ts...>(*str, name, section);
            else
                return def_vals;
        }

        // Read if value exist. Return default value if not, assert if section doesn't exists
        template <typename T>
        static IA force_read_ive(StrViewCref name, const T& def_val) {
            return force_read_ive<T>(name, cfg_detls::GLOBAL_NAMESPACE, def_val);
        }
        template <typename T1, typename T2, typename... Ts>
        static IA force_read_ive(StrViewCref name, const std::tuple<T1, T2, Ts...>& def_vals) {
            return force_read_ive<T1, T2, Ts...>(name, cfg_detls::GLOBAL_NAMESPACE, def_vals);
        }
        template <typename T>
        static T force_read_ive(StrViewCref name, StrViewCref section, const T& def_val) {
            auto sect = cfg_detls::cfgData().getSection(String(section));
            auto str  = sect.valueOpt(String(name));
            if (str)
                return superCast<T>(*str, name, section);
            else
                return def_val;
        }
        template <typename T1, typename T2, typename... Ts>
        static auto force_read_ive(StrViewCref name, StrViewCref section, const std::tuple<T1, T2, Ts...>& def_vals) {
            auto sect = cfg_detls::cfgData().getSection(String(section));
            auto str  = sect.valueOpt(String(name));
            if (str)
                return superCast<T1, T2, Ts...>(*str, name, section);
            else
                return def_vals;
        }

        template <typename T>
        static T force_read(StrViewCref name, StrViewCref section = cfg_detls::GLOBAL_NAMESPACE) {
            return superCast<T>(readString(name, section), name, section);
        }

        template <typename T1, typename T2, typename... Ts>
        static auto force_read(StrViewCref name, StrViewCref section = cfg_detls::GLOBAL_NAMESPACE) {
            auto str = readString(name, section);
            auto vec = readerUnpackMulti(name, section, str, sizeof...(Ts) + 2);
            auto is  = std::make_index_sequence<sizeof...(Ts) + 2>();

            return readTupleImpl<T1, T2, Ts...>(vec, name, section, is);
        }


    private:

        // Basic read method
        SIA readString(StrViewCref name, StrViewCref section) -> StringRef {
            return cfg_detls::cfgData().getValue(String(section), String(name));
        }

        //
        template <typename... Ts, SizeT... _Idx>
        SIA readTupleImpl(StrvVector& vec, StrViewCref name, StrViewCref section, std::index_sequence<_Idx...>) {
            return std::make_tuple(superCast<Ts>(vec[_Idx], name, section)...);
        }

        template <typename T, SizeT... _Idx>
        SIA readArrayImpl(StrvVector& vec, StrViewCref name, StrViewCref section, std::index_sequence<_Idx...>) {
            auto arr = ScmArray<T, sizeof...(_Idx)>{};

            ((arr[_Idx] = superCast<T>(vec[_Idx], name, section)) , ...);

            return arr;
        }

        template <typename T>
        SIA readVectorImpl(StrvVector& vec, StrViewCref name, StrViewCref section) {
            auto res = ScmVector<T>{};

            for (auto& s : vec)
                res.push_back(superCast<T>(s, name, section));

            return res;
        }

        // Numbers
        template <typename T>
        SIA superCast(StrViewCref str, StrViewCref, StrViewCref)
        -> std::enable_if_t<scm_utils::numbers<T>, T> {
            auto nullTerminatedStr = ScmString(str);
            return scm_aton::aton<T>(nullTerminatedStr.c_str());
        }

        // String :)
        template <typename T>
        SIA superCast(StrViewCref str, StrViewCref, StrViewCref)
        -> std::enable_if_t<scm_utils::any_of<T, ScmStrView, ScmStrView, std::string_view, std::string>, T> {
            return T(str);
        }

        template <typename T>
        SIA superCast(StrViewCref str, StrViewCref name, StrViewCref section)
        -> std::enable_if_t<std::is_same_v<T, bool>, bool> {
            if (str == "true" || str == "on")
                return true;
            else if (str == "false" || str == "off")
                return false;
            else
                #ifdef SCM_ASSERTS
                    SCM_ASSERTS("Unknown bool value '%s' at key '%s' in section [%s].",
                            str.data(), name.data(), section.data());
                #elif defined SCM_FMT_ASSERTS
                    SCM_FMT_ASSERTS("Unknown bool value '{}' at key '{}' in section [{}].", str, name, section);
                #else
                    throw CfgException(cfg_detls::str_join(
                            String("Unknown bool value '"), str, "' at key '",
                            name.data(), "' in section [", section.data(), "]."));
                #endif
            return false; // !?
        }

        // Array
        template <typename A, typename T = std::remove_reference_t<decltype(std::declval<A>()[0])>, SizeT _Size = sizeof(A)/sizeof(T)>
        SIA superCast(StrViewCref str, StrViewCref name, StrViewCref section)
        -> std::enable_if_t<scm_utils::any_of<A, ScmArray<T, _Size>, std::array<T, _Size>>, A> {
            auto vec = readerUnpackMulti(name, section, str, _Size);
            return readArrayImpl<T>(vec, name, section, std::make_index_sequence<_Size>());
        }

        // Vector
        template <typename A, typename T = std::remove_reference_t<decltype(std::declval<A>()[0])>>
        SIA superCast(StrViewCref str, StrViewCref name, StrViewCref section)
        -> std::enable_if_t<scm_utils::any_of<A, ScmVector<T>, std::vector<T>>, A> {
            auto vec = readerUnpackMulti(name, section, str);
            return readVectorImpl<T>(vec, name, section);
        }


        static auto readerUnpackMulti
        (StrViewCref name, StrViewCref section, StrViewCref str, SizeT required = 0) -> StrvVector {
            StrvVector vec;

            auto ptr   = str.cbegin();

            while (ptr != str.cend()) {
                if (*ptr == '{') {
                    int entryLevel = 0;
                    auto start = ptr;

                    bool onSingleQuotes = false;
                    bool onDoubleQuotes = false;

                    for (; ptr != str.cend(); ++ptr) {
                        if (*ptr == '\'' && !onDoubleQuotes)
                            onSingleQuotes = !onSingleQuotes;

                        else if (*ptr == '\"' && !onSingleQuotes)
                            onDoubleQuotes = !onDoubleQuotes;

                        else if (!onSingleQuotes && !onDoubleQuotes) {
                            if (*ptr == '{') {
                                ++entryLevel;
                            } else if (*ptr == '}') {
                                --entryLevel;

                                if (entryLevel == 0) {
                                    if (ptr + 1 != str.cend()) {
                                        #ifdef SCM_ASSERTS
                                            SCM_ASSERTS(*(ptr + 1) != '}',
                                                            "Redundant '}' at key '%s' in section [%s].",
                                                            String(name).data(), String(section).data());

                                                SCM_ASSERTS(*(ptr + 1) == ',',
                                                            "Missing ',' at key '%s' in section [%s].",
                                                            String(name).data(), String(section).data());
                                        #elif defined SCM_FMT_ASSERTS
                                            SCM_FMT_ASSERTS(*(ptr + 1) != '}',
                                                                "Redundant '{}' at key '{}' in section [{}].",
                                                                '}', name, section);

                                                SCM_FMT_ASSERTS(*(ptr + 1) == ',',
                                                                "Missing ',' at key '{}' in section [{}].",
                                                                name, section);
                                        #else
                                            if (*(ptr + 1) == '}')
                                                throw cfg_detls::CfgException(cfg_detls::str_join(
                                                        "Redundant '}' at key '", name, "' in section [", section, "]."));

                                            if (*(ptr + 1) != ',')
                                                throw cfg_detls::CfgException(cfg_detls::str_join(
                                                        "Missing ',' at key '", name, "' in section [", section, "]."));
                                        #endif
                                    }

                                    break;
                                }
                            }
                        }
                    }

                    #ifdef SCM_ASSERTS
                        SCM_ASSERTS(entryLevel == 0,
                                        "Missing close '}' at key '%s' in section [%s].",
                                        String(name).data(), String(section).data());
                    #elif defined SCM_FMT_ASSERTS
                        SCM_FMT_ASSERTS(entryLevel == 0,
                                            "Missing close '{}' at key '{}' in section [{}].",
                                            '}', name, section);
                    #else
                        if (entryLevel != 0)
                            throw cfg_detls::CfgException(cfg_detls::str_join(
                                    "Missing close '}' at key '", name, "' in section [", section, "]."));
                    #endif

                    if (*ptr == '}' && ptr != str.cend())
                        ++ptr;


                    vec.emplace_back(str.substr(start - str.cbegin(), ptr - start));

                    if (ptr != str.cend())
                        ++ptr;

                }
                else {
                    auto start = ptr;

                    bool onSingleQuotes = false;
                    bool onDoubleQuotes = false;

                    for (; ptr != str.cend(); ++ptr) {
                        if (*ptr == '\'' && !onDoubleQuotes)
                            onSingleQuotes = !onSingleQuotes;

                        else if (*ptr == '\"' && !onSingleQuotes)
                            onDoubleQuotes = !onDoubleQuotes;

                        else if (!onSingleQuotes && !onDoubleQuotes) {
                            if (*ptr == ',')
                                break;

                            #ifdef SCM_ASSERTS
                                SCM_ASSERTS(cfg_detls::validate_keyval(*ptr),
                                                "Undefined char symbol '%c' at key '%s' in section [%s].",
                                                *ptr, String(name).data(), String(section).data());
                            #elif defined SCM_FMT_ASSERTS
                                SCM_FMT_ASSERTS(cfg_detls::validate_keyval(*ptr),
                                                    "Undefined char symbol '{}' at key '{}' in section [{}].",
                                                    *ptr, name, section);
                            #else
                                if (!cfg_detls::validate_keyval(*ptr))
                                    throw cfg_detls::CfgException(cfg_detls::str_join(
                                            "Undefined char symbol '", String(1, *ptr), "' at key '", name,
                                            "' in section [", section, "]."));
                            #endif
                        }
                    }

                    vec.emplace_back(str.substr(start - str.cbegin(), ptr - start));

                    if (ptr != str.cend())
                        ++ptr;
                }
            }

            if (vec.size() == 1 && vec[0].size() > 1 && vec[0].front() == '{' && vec[0].back() == '}')
                return readerUnpackMulti(name, section, vec[0].substr(1, vec[0].size() - 2), required);

            #ifdef SCM_ASSERTS
                SCM_ASSERTS(required == 0 || required == vec.size(),
                                "Wrong number of values at key '%s' in section [%s]. Provided %z, required %z.",
                                String(name).data(), String(section).data(), vec.size(), required);
            #elif defined SCM_FMT_ASSERTS
                SCM_FMT_ASSERTS(required == 0 || required == vec.size(),
                                    "Wrong number of values at key '{}' in section [{}]. Provided {}, required {}.",
                                    name, section, vec.size(), required);
            #else
                if (required != 0 && required != vec.size())
                    throw cfg_detls::CfgException(cfg_detls::str_join(
                            "Wrong number of values at key '", name, "' in section [", section, "]. Provided ",
                            std::to_string(vec.size()).data(), ", required ", std::to_string(required).data(), "."));
            #endif

            return std::move(vec);
        }

        void load() {
            cfg_detls::processFileTask();
            cfg_detls::cfgData().reloadParents();
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
            cfg_detls::cfg_state()._onCreate  = true;
            load();
            cfg_detls::cfg_state()._onCreate  = false;
            cfg_detls::cfg_state()._isCreated = true;
        }
        ~ConfigManager() = default;
    };


    using StrViewCref = const ScmStrView&;

    /**
     * Add entry file to loading
     * @param path - path to config entry file
     */
    inline void addCfgEntry(ScmString& path) {
        cfg_detls::cfg_state().addCfgEntry(path);
    }

    /**
     * Remove all entry files
     */
    inline void resetCfgEntries() {
        cfg_detls::cfg_state().clearCfgEntries();
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
    IA read(StrViewCref name, StrViewCref section = cfg_detls::GLOBAL_NAMESPACE) {
        return cfg().read<Ts...>(name, section);
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
        return cfg().read_ie<T>(name, def_val);
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
        return cfg().read_ie<T1, T2, Ts...>(name, def_vals);
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
        return cfg().read_ie<T>(name, section, def_val);
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
        return cfg().read_ie<T1, T2, Ts...>(name, section, def_vals);
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
        return cfg().read_ive<T>(name, def_val);
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
        return cfg().read_ive<T1, T2, Ts...>(name, def_vals);
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
        return cfg().read_ive<T>(name, section, def_val);
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
        return cfg().read_ive<T1, T2, Ts...>(name, section, def_vals);
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
        if (cfg_detls::cfg_state().onCreate())
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
        if (cfg_detls::cfg_state().onCreate())
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
        if (cfg_detls::cfg_state().onCreate())
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
        if (cfg_detls::cfg_state().onCreate())
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
        if (cfg_detls::cfg_state().onCreate())
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
        if (cfg_detls::cfg_state().onCreate())
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
        if (cfg_detls::cfg_state().onCreate())
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
        if (cfg_detls::cfg_state().onCreate())
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
            const std::string_view &section = cfg_detls::GLOBAL_NAMESPACE) {
        if (cfg_detls::cfg_state().onCreate())
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
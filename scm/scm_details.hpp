#pragma once

#include "scm_utils.hpp"
#include "scm_types.hpp"
#include "scm_filesystem.hpp"
#include "scm_aton.hpp"

namespace scm_details {
    using SizeT       = ScmSizeT;

    using String      = ScmString;
    using StringCref  = const String&;
    using StringRef   = String&;
    using StringRval  = String&&;

    using StrView     = ScmStrView;
    using StrViewCref = const StrView&;
    using StrViewRef  = StrView&;
    using StrViewRval = StrView&&;

    using StrViewPair   = ScmPair<StrView, StrView>;
    using StrVector     = ScmVector<String>;
    using StrViewVector = ScmVector<StrView>;
    using StrStrMap     = ScmMap<String, String>;
    using StrSectionMap = ScmMap<String, class Section>;
    using CfgException  = SCM_NAMESPACE::CfgException;

    static constexpr inline std::string_view GLOBAL_NAMESPACE = "__global";

    void parse (StrViewCref path);

    auto unpack(StrViewCref name, StrViewCref section, StrViewCref str, SizeT required) -> StrViewVector;

    ////////////////////////////////// Section /////////////////////////////////

    class Section {
    public:
        auto getValue(StringCref key) const -> StringCref {
            auto val = _pairs.find(key);

            SCM_EXCEPTION(CfgException, val != _pairs.end(), "Can't find key '", key, "' in section [", _name, "]");

            return val->second;
        }

        auto getValue(StringCref key) -> StringRef {
            auto val = _pairs.find(key);

            SCM_EXCEPTION(CfgException, val != _pairs.end(), "Can't find key '", key, "' in section [", _name, "]");

            return val->second;
        }

        auto value(StringCref key) -> StringRef {
            return _pairs[key];
        }

        auto valueOpt(StringCref key) const -> std::optional<String>  {
            auto val = _pairs.find(key);
            if (val == _pairs.end())
                return {};
            else
                return val->second;
        }

        bool isExists  (StringCref key) const {
            return _pairs.find(key) != _pairs.end();
        }

        void add       (StringCref key, StringCref value) { _pairs.emplace(key, value); }
        void add       (StringCref key, StringRval value) { _pairs.emplace(key, value); }
        void addParent (StringCref parent)                { _parents.push_back(parent); }

        auto getMap     () -> StrStrMap&             { return _pairs; }
        auto getMap     () const -> const StrStrMap& { return _pairs; }
        auto getParents () -> StrVector&             { return _parents; }
        auto getParents () const -> const StrVector& { return _parents; }

        auto& name()       { return _name; }
        auto& name() const { return _name; }

        void reload();

    private:
        StrStrMap _pairs;
        StrVector _parents;
        String    _name;
        String    _file;
        bool      _alreadyReloaded = false;
    };


    ////////////////////////// Config Data Storage /////////////////////////////

    class CfgData {
    public:
        auto getSection(StringCref key) const  -> const Section& {
            auto sect = _sections.find(key);

            SCM_EXCEPTION(CfgException, sect != _sections.end(), "Can't find section [", key, "]");

            return sect->second;
        }

        auto getSection(StringCref key) -> Section& {
            auto sect = _sections.find(key);

            SCM_EXCEPTION(CfgException, sect != _sections.end(), "Can't find section [", key, "]");

            return sect->second;
        }

        auto section(StringCref key) -> Section& {
            auto& sect = _sections[key];

            if (sect.name().empty())
                sect.name() = key;

            return sect;
        }

        auto sectionOpt(StringCref key) const -> std::optional<Section> {
            auto sect = _sections.find(key);
            if (sect == _sections.end())
                return {};
            else
                return sect->second;
        }

        bool isSectionExists(StringCref key) const {
            return _sections.find(key) != _sections.end();
        }

        auto addSection(StringCref path, SizeT lineNum, StringCref key) -> Section& {
            SCM_EXCEPTION(CfgException, !isSectionExists(key) || key == GLOBAL_NAMESPACE,
                          "Duplicate section [", key, "] in ", path, ":", std::to_string(lineNum + 1).data());

            auto& sect = _sections[key];
            sect.name() = key;

            return sect;
        }

        auto getValue(StringCref sect, StringCref key) const -> StringCref {
            return getSection(sect).getValue(key);
        }

        auto getValue(StringCref sect, StringCref key) -> StringRef {
            return getSection(sect).getValue(key);
        }

        auto value(StringCref sect, StringCref key) -> StringRef {
            return section(sect).value(key);
        }

        auto valueOpt(StringCref section, StringCref key) const -> std::optional<String> {
            auto res1 = sectionOpt(section);
            if (res1)
                return res1->valueOpt(key);
            return {};
        }

        bool isValueExists(StringCref sect, StringCref key) const {
            auto res1 = isSectionExists(sect);
            if (res1)
                return getSection(sect).isExists(key);
            return false;
        }

        void addValue(StringCref sect, StringCref key, StringCref value) {
            section(sect).add(key, value);
        }
        void addValue(StringCref sect, StringCref key, StringRval value) {
            section(sect).add(key, value);
        }

        void reloadParents() {
            for (auto& s : _sections)
                s.second.reload();
        }

        void clear() {
            _sections.clear();
        }

    private:
        StrSectionMap _sections;

        // Singleton impl
    public:
        CfgData(const CfgData&) = delete;
        CfgData& operator= (const CfgData&) = delete;

        static CfgData& instance() {
            static CfgData inst;
            return inst;
        }

    private:
        CfgData()  = default;
        ~CfgData() = default;
    };

    inline CfgData& cfg_data() { return CfgData::instance(); }



    void Section::reload() {
        for (const auto& sectStr : _parents) {
            auto& sect = cfg_data().getSection(sectStr);

            if (!sect._alreadyReloaded && !sect.getParents().empty())
                sect.reload();

            auto pairs = sect.getMap();

            for (const auto& pair : pairs) {
                if (!isExists(pair.first))
                    add(pair.first, pair.second);
            }
        }

        _alreadyReloaded = true;
    }

    // Numbers
    template <typename T>
    auto superCast(StrViewCref str, StrViewCref name, StrViewCref section)
    -> std::enable_if_t<SCM_NAMESPACE::numbers<T>, T> {
        return SCM_NAMESPACE::aton<T>(str, name, section);
    }

    // String :)
    template <typename T>
    auto superCast(StrViewCref str, StrViewCref, StrViewCref)
    -> std::enable_if_t<SCM_NAMESPACE::any_of<T, ScmString, ScmStrView, std::string_view, std::string>, T> {
        return T(str);
    }

    template <typename T>
    auto superCast(StrViewCref str, StrViewCref name, StrViewCref section)
    -> std::enable_if_t<std::is_same_v<T, bool>, bool> {
        if (str == "true" || str == "on")
            return true;
        else if (str == "false" || str == "off")
            return false;
        else
            SCM_EXCEPTION(CfgException, 0, "Unknown bool value '", str, "' at key '",
                          name.data(), "' in section [", section.data(), "].");
        return false; // !?
    }

    //
    template <typename... Ts, SizeT... _Idx>
    auto readTupleImpl(StrViewVector& vec, StrViewCref name, StrViewCref section, std::index_sequence<_Idx...>) {
        return std::make_tuple(superCast<Ts>(vec[_Idx], name, section)...);
    }

    template <typename T, SizeT... _Idx>
    auto readArrayImpl(StrViewVector& vec, StrViewCref name, StrViewCref section, std::index_sequence<_Idx...>) {
        auto arr = ScmArray<T, sizeof...(_Idx)>{};

        ((arr[_Idx] = superCast<T>(vec[_Idx], name, section)) , ...);

        return arr;
    }

    template <typename T>
    auto readVectorImpl(StrViewVector& vec, StrViewCref name, StrViewCref section) {
        auto res = ScmVector<T>{};

        for (auto& s : vec)
            res.push_back(superCast<T>(s, name, section));

        return res;
    }

    // Array
    template <typename A, typename T, SizeT _Size>
    auto superCast(StrViewCref str, StrViewCref name, StrViewCref section)
    -> std::enable_if_t<SCM_NAMESPACE::any_of<A, ScmArray<T, _Size>, std::array<T, _Size>>, A> {
        auto vec = unpack(name, section, str, _Size);
        return readArrayImpl<T>(vec, name, section, std::make_index_sequence<_Size>());
    }

    template <typename T, SizeT... _Idx>
    auto readTupleTuple(StrViewCref str, StrViewCref name, StrViewCref section, std::index_sequence<_Idx...>) {
        auto vec = unpack(name, section, str, sizeof...(_Idx));
        return std::make_tuple(
                superCast<SCM_NAMESPACE::remove_const_ref<
                             decltype(std::get<_Idx>(std::declval<T>()))
                          >>(vec[_Idx], name, section)...);
    }

    // Tuple
    template <typename T>
    auto superCast(StrViewCref str, StrViewCref name, StrViewCref section)
    -> std::enable_if_t<SCM_NAMESPACE::Is_specialization_of<T, std::tuple>::value, T> {
        return readTupleTuple<T>(str, name, section, std::make_index_sequence<std::tuple_size_v<T>>());
    }

    // Pair
    template <typename T>
    auto superCast(StrViewCref str, StrViewCref name, StrViewCref section)
    -> std::enable_if_t<
            SCM_NAMESPACE::Is_specialization_of<T, std::pair>::value ||
            SCM_NAMESPACE::Is_specialization_of<T, ScmPair>::value, T>
    {
        auto vec = unpack(name, section, str, 2);
        return T(superCast<decltype(std::declval<T>().first)>(vec[0], name, section),
                 superCast<decltype(std::declval<T>().first)>(vec[1], name, section));
    }

    // Vector
    template <typename A, typename T>
    auto superCast(StrViewCref str, StrViewCref name, StrViewCref section)
    -> std::enable_if_t<SCM_NAMESPACE::any_of<A, ScmVector<T>, std::vector<T>>, A> {
        auto vec = unpack(name, section, str, 0);
        return readVectorImpl<T>(vec, name, section);
    }

    template <typename T>
    constexpr bool no_str_view_or_c_array =
            !SCM_NAMESPACE::any_of<T, StrView, std::string_view> && !SCM_NAMESPACE::is_c_array<T>;
} // namespace scm_details


#ifndef SCM_NO_INLINE
    #include "scm_details_inl.hpp"
#endif
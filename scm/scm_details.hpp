#pragma once

#include "scm_utils.hpp"
#include "scm_types.hpp"
#include "scm_filesystem.hpp"

#ifdef SCM_NAMESPACE
namespace SCM_NAMESPACE {
#endif // SCM_NAMESPACE

    class ConfigManager;

#ifdef SCM_NAMESPACE
}
#endif // SCM_NAMESPACE


//////////////////////////////////// Details ///////////////////////////////////

namespace cfg_detls {
    // Typedefs

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


    // Constants

    static constexpr inline std::string_view GLOBAL_NAMESPACE = "__global";

    static inline auto DEFAULT_CFG_PATH() {
        return
            scm_utils::append_path(
                #ifdef SCM_NAMESPACE
                    scm_utils::parent_path(SCM_NAMESPACE::fs::current_path()),
                #else
                    scm_utils::parent_path(fs::current_path()),
                #endif
                ScmString("fs.cfg"));
    }


    // Cfg exception

    class CfgException : public std::exception {
    public:
        CfgException(String error) : _exc(std::move(error)) {}
        const char* what() const noexcept override { return _exc.data(); }

    private:
        String _exc;
    };


    // Helper function for strings
    template <typename ... ArgT>
    static auto str_join(const ArgT& ... strs) -> ScmString {
        return (String(strs) + ...);
    }


    // Creation state

    class CfgCreationState {
        friend
        #ifdef SCM_NAMESPACE
                SCM_NAMESPACE::
        #endif // SCM_NAMESPACE
                                ConfigManager;
    public:
        void clearCfgEntries() {
            _cfgEntries.clear();
        }

        void addCfgEntry(StringCref path) {
            _cfgEntries.push_back(path);
        }

        bool onCreate () const { return _onCreate; }
        bool isCreated() const { return _isCreated; }

        const auto& getEntries() const {
            return _cfgEntries;
        }

    private:
        bool _onCreate  = false;
        bool _isCreated = false;

        StrVector _cfgEntries;

        // Singleton impl
    public:
        CfgCreationState(const CfgCreationState&) = delete;
        CfgCreationState(CfgCreationState&&) = delete;
        CfgCreationState& operator=(const CfgCreationState&) = delete;

        static CfgCreationState& instance() {
            static CfgCreationState inst;
            return inst;
        }

    private:
        CfgCreationState () {
            // Add default cfgPath
            _cfgEntries.emplace_back(DEFAULT_CFG_PATH());
        }
        ~CfgCreationState() = default;
    };


    inline CfgCreationState& cfg_state() {
        return CfgCreationState::instance();
    }



    ////////////////////////////////// Section /////////////////////////////////

    class Section {
    public:
        auto getValue(StringCref key) const -> StringCref {
            auto val = _pairs.find(key);

            #ifdef SCM_ASSERTS
                SCM_ASSERTS(val != _pairs.end(), "Can't find key '%s' in section [%s]", key.data(), _name.data());
            #elif defined SCM_FMT_ASSERTS
                SCM_FMT_ASSERTS(val != _pairs.end(), "Can't find key '{}' in section [{}]", key, _name);
            #else
                if (val == _pairs.end())
                    throw CfgException(cfg_detls::str_join(
                            "Can't find key '", key, "' in section [", _name, "]"));
            #endif

            return val->second;
        }

        auto getValue(StringCref key) -> StringRef {
            auto val = _pairs.find(key);

            #ifdef SCM_ASSERTS
                SCM_ASSERTS(val != _pairs.end(), "Can't find key '%s' in section [%s]", key.data(), _name.data());
            #elif defined SCM_FMT_ASSERTS
                SCM_FMT_ASSERTS(val != _pairs.end(), "Can't find key '{}' in section [{}]", key, _name);
            #else
                if (val == _pairs.end())
                    throw CfgException(cfg_detls::str_join(
                            "Can't find key '", key, "' in section [", _name, "]"));
            #endif

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

        void add(StringCref key, StringCref value) {
            _pairs.emplace(key, value);
        }

        void add(StringCref key, StringRval value) {
            _pairs.emplace(key, value);
        }

        void addParent(StringCref parent) {
            _parents.push_back(parent);
        }

        auto getMap    () -> StrStrMap&             { return _pairs; }
        auto getMap    () const -> const StrStrMap& { return _pairs; }
        auto getParents() -> StrVector&             { return _parents; }
        auto getParents() const -> const StrVector& { return _parents; }

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

            #ifdef SCM_ASSERTS
                SCM_ASSERTS(sect != _sections.end(), "Can't find section [%s]", key.data());
            #elif defined SCM_FMT_ASSERTS
                SCM_FMT_ASSERTS(sect != _sections.end(), "Can't find section [{}]", key);
            #else
                if (sect == _sections.end())
                    throw CfgException(cfg_detls::str_join(
                            "Can't find section [", key, "]"));
            #endif

            return sect->second;
        }

        auto getSection(StringCref key) -> Section& {
            auto sect = _sections.find(key);

            #ifdef SCM_ASSERTS
                SCM_ASSERTS(sect != _sections.end(), "Can't find section [%s]", key.data());
            #elif defined SCM_FMT_ASSERTS
                SCM_FMT_ASSERTS(sect != _sections.end(), "Can't find section [{}]", key);
            #else
                if (sect == _sections.end())
                    throw CfgException(cfg_detls::str_join(
                            "Can't find section [", key, "]"));
            #endif

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
            #ifdef SCM_ASSERTS
                SCM_ASSERTS(!isSectionExists(key) || !key.compare(GLOBAL_NAMESPACE),
                            "Duplicate section [%s] in %s:%z", key.data(), path.data(), lineNum + 1);
            #elif defined SCM_FMT_ASSERTS
                SCM_ASSERTS(!isSectionExists(key) || !key.compare(GLOBAL_NAMESPACE),
                            "Duplicate section [{}] in {}:{}", key, path, lineNum + 1);
            #else
                if (isSectionExists(key) && (key != GLOBAL_NAMESPACE))
                    throw CfgException(cfg_detls::str_join(
                            "Duplicate section [", key, "] in ", path, ":", std::to_string(lineNum + 1).data()));
            #endif

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

        //void _print_info() const;

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

    inline CfgData& cfgData() { return CfgData::instance(); }



    void Section::reload() {
        for (const auto& sectStr : _parents) {
            auto& sect = cfgData().getSection(sectStr);

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



    ////////////////////////// Private functions ///////////////////////////////

    inline bool is_plain_text(ScmChar8 c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
    }

    inline bool is_digit(ScmChar8 c) {
        return c >= '0' && c <= '9';
    }

    inline bool is_space(ScmChar8 c) {
        return c == ' ' || c == '\t';
    }

    inline bool is_bracket(ScmChar8 c) {
        return c == '\'' || c == '\"';
    }

    inline bool is_legal_name_symbol(ScmChar8 c) {
        return c == '@' || c == '.' || c =='/' || c == '\\' || c == '-';
    }

    inline bool is_symbol(ScmChar8 c) {
        return c == ',' || c == ';' || c == '#' || c == '[' || c == ']' ||
               c == '+' || c == '=' || c == '$' || c == '{' || c == '}' ||
               c == ':' || is_legal_name_symbol(c) || is_bracket(c);
    }

    inline bool validate_name_symbol(ScmChar8 c) {
        return is_plain_text(c) || is_digit(c) || is_legal_name_symbol(c);
    }

    inline bool validate_keyval(ScmChar8 c) {
        return validate_name_symbol(c) || is_space(c) || c == '-' || c == '+';
    }

    inline bool validate_symbol(ScmChar8 c) {
        return is_space(c) || is_digit(c) || is_plain_text(c) || is_symbol(c);
    }

    // Skip spaces, return true if end passed
    template <typename IterT>
    inline bool skip_spaces_if_no_endl(IterT& ptr, IterT end) {
        while(is_space(*ptr) && ptr != end)
            ++ptr;

        return ptr == end;
    }

    // Skip spaces, return true if beg passed
    template <typename IterT>
    inline bool rskip_spaces_if_no_beg(IterT& ptr, IterT beg) {
        while(is_space(*ptr) && ptr != beg)
            --ptr;

        return ptr == beg;
    }

    inline auto remove_space_bounds_if_exists(StrViewCref str) -> StrView {
        auto start = str.cbegin();
        auto last  = str.cend() - 1;

        if (!skip_spaces_if_no_endl(start, last)) {
            if (!rskip_spaces_if_no_beg(last, start))
                return str.substr(start - str.cbegin(), last - start + 1);
        }

        return str;
    }

    inline auto remove_brackets_if_exists(StrViewCref str) -> StrView {
        if (str.length() > 1 && is_bracket(str.front()) && str.back() == str.front())
            return str.substr(1, str.length() - 2);
        return str;
    }


    void deleteComments  (StringCref path, StrViewVector& lines);
    void processFileTask (StringCref path);
    void processFileTask ();
    void parseLinesTask  (StringCref path, StrViewVector& lines);
    void preprocessorTask(StringCref path, SizeT lineNum, StrViewCref line);
    auto pairFromLine    (StringCref path, SizeT lineNum, StrViewCref line) -> StrViewPair;
    auto unpackVariable  (StringCref path, SizeT lineNum, StrViewCref line) -> String;


    auto unpackVariable (StringCref path, SizeT lineNum, StrViewCref line) -> String {
        auto ptr = line.begin();

        bool onSingleQuotes = false;
        bool onDoubleQuotes = false;

        auto result = String();
        result.reserve(line.length());

        for (; ptr != line.cend();) {
            if (*ptr == '\'' && !onDoubleQuotes)
                onSingleQuotes = !onSingleQuotes;

            else if (*ptr == '\"' && !onSingleQuotes)
                onDoubleQuotes = !onDoubleQuotes;

            else if (onSingleQuotes || onDoubleQuotes) {
                result.push_back(*ptr);
            }
            else {
                if (*ptr == '$') {
                    ++ptr; // Skip '$'

                    ////////// Read key

                    #ifdef SCM_ASSERTS
                        SCM_ASSERTS(!skip_spaces_if_no_endl(ptr, line.cend()),
                                    "Empty key after '$' in %s:%z",
                                    path.data(), lineNum + 1);

                        SCM_ASSERTS(!is_digit(*ptr) && !is_legal_name_symbol(*ptr),
                                    "Starting key with symbol '%c' in %s:%z",
                                    *ptr, path.data(), lineNum + 1);
                    #elif defined SCM_FMT_ASSERTS
                        SCM_FMT_ASSERTS(!skip_spaces_if_no_endl(ptr, line.cend()),
                                        "Empty key after '$' in {}:{}",
                                        path, lineNum + 1);

                        SCM_FMT_ASSERTS(!is_digit(*ptr) && !is_legal_name_symbol(*ptr),
                                        "Starting key with symbol '{}' in {}:{}",
                                        *ptr, path, lineNum + 1);
                    #else
                        if (skip_spaces_if_no_endl(ptr, line.cend()))
                            throw cfg_detls::CfgException(cfg_detls::str_join(
                                    "Empty key after '$' in ", path, ":", std::to_string(lineNum + 1).data()));

                        if (is_digit(*ptr) || is_legal_name_symbol(*ptr))
                            throw cfg_detls::CfgException(cfg_detls::str_join(
                                    "Starting key with symbol '", String(1, *ptr), "' in ", path, ":",
                                    std::to_string(lineNum + 1).data()));
                    #endif


                    auto start = ptr;

                    while(!is_space(*ptr) && *ptr != ':' && ptr != line.cend() && *ptr != '}' && *ptr != ',') {
                        #ifdef SCM_ASSERTS
                            SCM_ASSERTS(validate_name_symbol(*ptr),
                                        "Invalid character '%c' in key after '$' in %s:%z",
                                        *ptr, path.data(), lineNum + 1);
                        #elif defined SCM_FMT_ASSERTS
                            SCM_FMT_ASSERTS(validate_name_symbol(*ptr),
                                            "Invalid character '{}' in key after '$' in {}:{}",
                                            *ptr, path, lineNum + 1);
                        #else
                            if (!validate_name_symbol(*ptr))
                                throw cfg_detls::CfgException(cfg_detls::str_join(
                                        "Invalid character '", String(1, *ptr), "' in key after '$' in ", path, ":",
                                        std::to_string(lineNum + 1).data()));
                        #endif

                        ++ptr;
                    }

                    auto val   = StrView();
                    auto first = line.substr(start - line.cbegin(), ptr - start);

                    skip_spaces_if_no_endl(ptr, line.cend());

                    ////////// Dereference key
                    if (ptr == line.cend() || *ptr != ':')
                        //////// Read value from global namespaces
                        val = cfg_detls::cfgData().getValue(String(cfg_detls::GLOBAL_NAMESPACE), String(first));
                    else if (*ptr == ':'){
                        //////// Read value from section (only no-parents section supported)
                        ++ptr; // skip ':'

                        #ifdef SCM_ASSERTS
                            SCM_ASSERTS(!skip_spaces_if_no_endl(ptr, line.cend()),
                                        "Empty key after ':' in %s:%z", path.data(), lineNum + 1);

                            SCM_ASSERTS(!is_digit(*ptr) && !is_legal_name_symbol(*ptr),
                                        "Starting key with symbol '%c' in %s:%z",
                                        *ptr, path.data(), lineNum + 1);
                        #elif defined SCM_FMT_ASSERTS
                            SCM_FMT_ASSERTS(!skip_spaces_if_no_endl(ptr, line.cend()),
                                            "Empty key after ':' in {}:{}", path, lineNum + 1);

                            SCM_FMT_ASSERTS(!is_digit(*ptr) && !is_legal_name_symbol(*ptr),
                                            "Starting key with symbol '{}' in {}:{}",
                                            *ptr, path, lineNum + 1);
                        #else
                            if (skip_spaces_if_no_endl(ptr, line.cend()))
                                throw cfg_detls::CfgException(cfg_detls::str_join(
                                        "Empty key after ':' in ", path, ":",
                                        std::to_string(lineNum + 1).data()));

                            if (is_digit(*ptr) || is_legal_name_symbol(*ptr))
                                throw cfg_detls::CfgException(cfg_detls::str_join(
                                        "Starting key with symbol '", String(1, *ptr), "' in ", path, ":",
                                        std::to_string(lineNum + 1).data()));
                        #endif

                        auto start2 = ptr;

                        while(!is_space(*ptr) && *ptr != ':' && ptr != line.cend()) {

                            #ifdef SCM_ASSERTS
                                SCM_ASSERTS(validate_name_symbol(*ptr),
                                            "Invalid character '%c' in key after '$' in %s:%z",
                                            *ptr, path.data(), lineNum + 1);
                            #elif defined SCM_FMT_ASSERTS
                                SCM_FMT_ASSERTS(validate_name_symbol(*ptr),
                                                "Invalid character '{}' in key after '$' in {}:{}",
                                                *ptr, path, lineNum + 1);
                            #else
                                if (!validate_name_symbol(*ptr))
                                    throw cfg_detls::CfgException(cfg_detls::str_join(
                                            "Invalid character '", String(1, *ptr), "' in key after '$' in ", path, ":",
                                            std::to_string(lineNum + 1).data()));
                            #endif

                            ++ptr;
                        }

                        auto second = line.substr(start2 - line.cbegin(), ptr - start2);

                        #ifdef SCM_ASSERTS
                            SCM_ASSERTS(cfg_detls::cfgData().getSection(first).getParents().empty(),
                                        "Attempt to dereference key '%s' from section [%s] with parent "
                                        "in %s:%z", String(second).data(), String(first).data(), path.data(), lineNum + 1);
                        #elif defined SCM_FMT_ASSERTS
                            SCM_FMT_ASSERTS(cfg_detls::cfgData().getSection(first).getParents().empty(),
                                            "Attempt to dereference key '{}' from section [{}] with parent "
                                            "in {}:{}", second, first, path, lineNum + 1);
                        #else
                            if (!cfg_detls::cfgData().getSection(String(first)).getParents().empty())
                                throw cfg_detls::CfgException(cfg_detls::str_join(
                                        "Attempt to dereference key '", second, "' from section [", first, "] with parent ",
                                        "in ", path, ":", std::to_string(lineNum + 1).data()));
                        #endif
                        val = cfg_detls::cfgData().getValue(String(first), String(second));
                    }

                    result += remove_brackets_if_exists(val);
                    continue;
                }
                else if (!is_space(*ptr)) {
                    result.push_back(*ptr);
                }
            }

            ++ptr;
        }

        return std::move(result);
    }


    auto pairFromLine(StringCref path, SizeT lineNum, StrViewCref line) -> StrViewPair {
        auto ptr = line.cbegin();

        if (skip_spaces_if_no_endl(ptr, line.cend()))
            return {};

        /////////// Read name

        auto start = ptr;

        #ifdef SCM_ASSERTS
            SCM_ASSERTS(!is_digit(*ptr) && !is_legal_name_symbol(*ptr),
                    "Starting key with symbol '%c' in %s:%z",
                    *ptr, path.data(), lineNum + 1);
        #elif defined SCM_FMT_ASSERTS
            SCM_FMT_ASSERTS(!is_digit(*ptr) && !is_legal_name_symbol(*ptr),
                            "Starting key with symbol '{}' in {}:{}",
                            *ptr, path, lineNum + 1);
        #else
            if (is_digit(*ptr) || is_legal_name_symbol(*ptr))
                throw cfg_detls::CfgException(cfg_detls::str_join(
                        "Starting key with symbol '", String(1, *ptr), "' in ", path, ":",
                        std::to_string(lineNum + 1).data()));
        #endif

        while(!is_space(*ptr) && *ptr != '=' && ptr != line.cend()) {
        #ifdef SCM_ASSERTS
                SCM_ASSERTS(validate_name_symbol(*ptr),
                            "Invalid character '%c' in key definition '$' in %s:%z",
                            *ptr, path.data(), lineNum + 1);
        #elif defined SCM_FMT_ASSERTS
                SCM_FMT_ASSERTS(validate_name_symbol(*ptr),
                                "Invalid character '{}' in key definition '$' in {}:{}",
                                *ptr, path, lineNum + 1);
        #else
                if (!validate_name_symbol(*ptr))
                    throw cfg_detls::CfgException(cfg_detls::str_join(
                            "Invalid character '", String(1, *ptr), "' in key definition '$' in ", path, ":",
                            std::to_string(lineNum + 1).data()));
        #endif

            ++ptr;
        }

        auto key = line.substr(start - line.cbegin(), ptr - start);


        //////////// Read values (no space deleting inside values)
        #ifdef SCM_ASSERTS
            SCM_ASSERTS(!skip_spaces_if_no_endl(ptr, line.cend()),
                        "Missing value at key '%s' in %s:%z", String(key).data(), path.data(), lineNum + 1);

            SCM_ASSERTS(*ptr == '=',
                        "Missing delimiter '=' at key '%s' in %s:%z", String(key).data(), path.data(), lineNum + 1);
        #elif defined SCM_FMT_ASSERTS
            SCM_FMT_ASSERTS(!skip_spaces_if_no_endl(ptr, line.cend()),
                            "Missing value at key '{}' in {}:{}", key, path, lineNum + 1);

            SCM_FMT_ASSERTS(*ptr == '=',
                            "Missing delimiter '=' at key '{}' in {}:{}", key, path, lineNum + 1);
        #else
            if (skip_spaces_if_no_endl(ptr, line.cend()))
                throw cfg_detls::CfgException(cfg_detls::str_join(
                        "Missing value at key '", key, "' in ", path, ":",
                        std::to_string(lineNum + 1).data()));

            if (*ptr != '=')
                throw cfg_detls::CfgException(cfg_detls::str_join(
                        "Missing delimiter '=' at key '", key, "' in ", path, ":",
                        std::to_string(lineNum + 1).data()));
        #endif

        ++ptr;

        auto value = remove_space_bounds_if_exists(line.substr(ptr - line.cbegin(), line.end() - ptr));

        #ifdef SCM_ASSERTS
            SCM_ASSERTS(!value.empty(),
                        "Missing value at key '%s' in %s:%z", String(key).data(), path.data(), lineNum + 1);
        #elif defined SCM_FMT_ASSERTS
            SCM_FMT_ASSERTS(!value.empty(),
                            "Missing value at key '{}' in {}:{}", key, path, lineNum + 1);
        #else
            if (value.empty())
                throw cfg_detls::CfgException(cfg_detls::str_join(
                        "Missing value at key '", key, "' in ", path, ":",
                        std::to_string(lineNum + 1).data()));
        #endif

        return StrViewPair(key, value);
    }


    void deleteComments(StringCref path, StrViewVector& lines) {
        for (SizeT n = 0; n < lines.size(); ++n) {
            bool onSingleQuotes = false;
            bool onDoubleQuotes = false;

            auto& line = lines[n];
            auto  i    = line.begin();

            for (; i != line.end(); ++i) {
                if (*i == '\'' && !onDoubleQuotes)
                    onSingleQuotes = !onSingleQuotes;

                else if (*i == '\"' && !onSingleQuotes)
                    onDoubleQuotes = !onDoubleQuotes;

                else if (!onSingleQuotes && !onDoubleQuotes) {
                    #ifdef SCM_ASSERTS
                        SCM_ASSERTS(validate_symbol(*i),
                                    "Undefined char symbol '%c' [%i] in %s:%z",
                                    *i, unsigned(*i), path.data(), n + 1);
                    #elif defined SCM_FMT_ASSERTS
                        SCM_FMT_ASSERTS(validate_symbol(*i),
                                        "Undefined char symbol '{}' [{}] in {}:{}",
                                        *i, unsigned(*i), path, n + 1);
                    #else
                        if (!validate_symbol(*i))
                            throw cfg_detls::CfgException(cfg_detls::str_join(
                                    "Undefined char symbol '", String(1, *i), "' [", std::to_string(unsigned(*i)).data(),
                                    "] in ", path, ":", std::to_string(n + 1).data()));
                    #endif

                    if (*i == ';' || (*i == '/' && *(i + 1) == '/')) {
                        line = line.substr(0, i - line.begin());
                        break;
                    }
                }
            }

            #ifdef SCM_ASSERTS
                SCM_ASSERTS(!onSingleQuotes, "Missing second \' quote in %s:%z", path.data(), n + 1);
                SCM_ASSERTS(!onDoubleQuotes, "Missing second \" quote in %s:%z", path.data(), n + 1);
            #elif defined SCM_FMT_ASSERTS
                SCM_FMT_ASSERTS(!onSingleQuotes, "Missing second \' quote in {}:{}", path, n + 1);
                SCM_FMT_ASSERTS(!onDoubleQuotes, "Missing second \" quote in {}:{}", path, n + 1);
            #else
                if (onSingleQuotes)
                    throw cfg_detls::CfgException(cfg_detls::str_join(
                            "Missing second \' quote in ", path, ":", std::to_string(n + 1).data()));
                if (onDoubleQuotes)
                    throw cfg_detls::CfgException(cfg_detls::str_join(
                            "Missing second \" quote in ", path, ":", std::to_string(n + 1).data()));
            #endif
        }
    }



    void preprocessorTask(StringCref path, SizeT lineNum, StrViewCref line) {
        auto ptr = line.cbegin();

        ++ptr; // skip '#'

        #ifdef SCM_ASSERTS
            SCM_ASSERTS(!skip_spaces_if_no_endl(ptr, line.cend()),
                        "Empty preprocessor directive in %s:%z", path.data(), lineNum + 1);
        #elif defined SCM_FMT_ASSERTS
            SCM_FMT_ASSERTS(!skip_spaces_if_no_endl(ptr, line.cend()),
                            "Empty preprocessor directive in {}:{}", path, lineNum + 1);
        #else
            if (skip_spaces_if_no_endl(ptr, line.cend()))
                throw cfg_detls::CfgException(cfg_detls::str_join(
                        "Empty preprocessor directive in ", path, ":", std::to_string(lineNum + 1).data()));
        #endif

        auto start = ptr;

        while (!is_space(*ptr) && ptr != line.cend()) {
        #ifdef SCM_ASSERTS
                SCM_ASSERTS(is_plain_text(*ptr),
                           "Invalid character in preprocessor directive in %s:%z",
                           path.data(), lineNum + 1);
        #elif defined SCM_FMT_ASSERTS
                SCM_FMT_ASSERTS(is_plain_text(*ptr),
                                "Invalid character in preprocessor directive in {}:{}",
                                path, lineNum + 1);
        #else
                if (!is_plain_text(*ptr))
                    throw cfg_detls::CfgException(cfg_detls::str_join(
                            "Invalid character in preprocessor directive in ", path, ":",
                            std::to_string(lineNum + 1).data()));
        #endif

            ++ptr;
        }

        auto first    = line.substr(start - line.cbegin(), ptr - start);
        auto backline = line.substr(ptr - line.cbegin(), line.cend() - ptr);

        if (first == "include") {
            auto appendPath = remove_brackets_if_exists(remove_space_bounds_if_exists(backline));

            #ifdef SCM_ASSERTS
                SCM_ASSERTS(!appendPath.empty(),
                           "Empty path in include directive in %s:%z", path.data(), lineNum + 1);
            #elif defined SCM_FMT_ASSERTS
                SCM_FMT_ASSERTS(!appendPath.empty(),
                                "Empty path in include directive in {}:{}", path, lineNum + 1);
            #else
                if (appendPath.empty())
                    throw cfg_detls::CfgException(cfg_detls::str_join(
                            "Empty path in include directive in ", path, ":",
                            std::to_string(lineNum + 1).data()));
            #endif

            processFileTask(scm_utils::append_path(scm_utils::parent_path(path), String(appendPath)));
        } else {
            #ifdef SCM_ASSERTS
                SCM_ASSERTS(0, "Unknown preprocessor directive '#%s' in %s:%z", String(first).data(), path.data(), lineNum + 1);
            #elif defined SCM_FMT_ASSERTS
                SCM_FMT_ASSERTS(0, "Unknown preprocessor directive '#{}' in {}:{}", first, path, lineNum + 1);
            #else
                throw cfg_detls::CfgException(cfg_detls::str_join(
                        "Unknown preprocessor directive '#", first, "' in ", path, ":",
                        std::to_string(lineNum + 1).data()));
            #endif
        }
    }



    void parseLinesTask(StringCref path, StrViewVector& lines) {
        using cfg_detls::Section;
        using cfg_detls::cfgData;

        Section* currentSection = nullptr;

        for (SizeT n = 0; n < lines.size(); ++n) {
            auto& line = lines[n];
            auto  ptr  = line.cbegin();

            if (skip_spaces_if_no_endl(ptr, line.cend()))
                continue;
            ////////////////////////////////////// Section /////////////////////////////////////////
            if (*ptr == '[') {
                auto start = ++ptr;

                ////////////// Read section

                #ifdef SCM_ASSERTS
                    SCM_ASSERTS(!is_digit(*ptr) && !is_legal_name_symbol(*ptr),
                               "Starting section with symbol '%c' in %s:%z",
                               *ptr, path.data(), n + 1);
                #elif defined SCM_FMT_ASSERTS
                    SCM_FMT_ASSERTS(!is_digit(*ptr) && !is_legal_name_symbol(*ptr),
                                    "Starting section with symbol '{}' in {}:{}",
                                    *ptr, path, n + 1);
                #else
                    if (is_digit(*ptr) || is_legal_name_symbol(*ptr))
                        throw cfg_detls::CfgException(cfg_detls::str_join(
                                "Starting section with symbol '", String(1, *ptr), "' in ", path, ":",
                                std::to_string(n + 1).data()));
                #endif

                while(*ptr != ']' && ptr != line.cend()) {
                    #ifdef SCM_ASSERTS
                        SCM_ASSERTS(validate_name_symbol(*ptr),
                                    "Invalid character '%c' in section definition in %s:%z",
                                    *ptr, path, n + 1);
                    #elif defined SCM_FMT_ASSERTS
                        SCM_FMT_ASSERTS(validate_name_symbol(*ptr),
                                        "Invalid character '{}' in section definition in {}:{}",
                                        *ptr, path, n + 1);
                    #else
                        if (!validate_name_symbol(*ptr))
                            throw cfg_detls::CfgException(cfg_detls::str_join(
                                    "Invalid character '", String(1, *ptr), "' in section definition in ", path, ":",
                                    std::to_string(n + 1).data()));
                    #endif

                    ++ptr;
                }

                #ifdef SCM_ASSERTS
                    SCM_ASSERTS(*ptr == ']', "Missing close section bracket in %s:%z", path.data(), n + 1);
                #elif defined SCM_FMT_ASSERTS
                    SCM_FMT_ASSERTS(*ptr == ']', "Missing close section bracket in {}:{}", path, n + 1);
                #else
                    if (*ptr != ']')
                        throw cfg_detls::CfgException(cfg_detls::str_join(
                                "Missing close section bracket in ", path, ":",
                                std::to_string(n + 1).data()));
                #endif

                currentSection =
                        &cfgData().addSection(path, n,
                                              String(line.substr(start - line.cbegin(), ptr - start)));

                ++ptr; // skip ']'

                if (skip_spaces_if_no_endl(ptr, line.cend()))
                    continue;


                /////////////// Read parents

                #ifdef SCM_ASSERTS
                    SCM_ASSERTS(*ptr == ':',
                                "Unexpected symbol '%c' after section [%s] definition in %s:%z.",
                                *ptr, currentSection->name().data(), path.data(), n+ 1);
                #elif defined SCM_FMT_ASSERTS
                    SCM_FMT_ASSERTS(*ptr == ':',
                                    "Unexpected symbol '{}' after section [{}] definition in {}:{}.",
                                    *ptr, currentSection->name(), path, n+ 1);
                #else
                    if (*ptr != ':')
                        throw cfg_detls::CfgException(cfg_detls::str_join(
                                "Unexpected symbol '", String(1, *ptr), "' after section [", currentSection->name(),
                                "] definition in ", path, ":", std::to_string(n + 1).data()));
                #endif

                ++ptr; // skip ':'

                #ifdef SCM_ASSERTS
                    SCM_ASSERTS(!skip_spaces_if_no_endl(ptr, line.cend()),
                                "Missing parents sections after ':' in %s:%z",
                                path.data(), n + 1);
                #elif defined SCM_FMT_ASSERTS
                    SCM_FMT_ASSERTS(!skip_spaces_if_no_endl(ptr, line.cend()),
                                    "Missing parents sections after ':' in {}:{}.",
                                    path, n + 1);
                #else
                    if (skip_spaces_if_no_endl(ptr, line.cend()))
                        throw cfg_detls::CfgException(cfg_detls::str_join(
                                "Missing parents sections after ':' in ", path, ":",
                                std::to_string(n + 1).data()));
                #endif

                while(ptr != line.end()) {
                    auto start2 = ptr;

                    #ifdef SCM_ASSERTS
                        SCM_ASSERTS(!is_digit(*ptr) && !is_legal_name_symbol(*ptr),
                                     "Starting parent definition with symbol '%c' in %s:%z",
                                     *ptr, path.data(), n + 1);
                    #elif defined SCM_FMT_ASSERTS
                        SCM_FMT_ASSERTS(!is_digit(*ptr) && !is_legal_name_symbol(*ptr),
                                        "Starting parent definition with symbol '{}' in {}:{}",
                                        *ptr, path, n + 1);
                    #else
                        if (is_digit(*ptr) || is_legal_name_symbol(*ptr))
                            throw cfg_detls::CfgException(cfg_detls::str_join(
                                    "Starting parent definition with symbol '", String(1, *ptr), "' in ", path, ":",
                                    std::to_string(n + 1).data()));
                    #endif

                    while(!is_space(*ptr) && ptr != line.cend() && *ptr != ',') {
                        #ifdef SCM_ASSERTS
                            SCM_ASSERTS(validate_name_symbol(*ptr),
                                        "Invalid character '%c' in parent definition in %s:%z",
                                        *ptr, path.data(), n + 1);
                        #elif defined SCM_FMT_ASSERTS
                            SCM_FMT_ASSERTS(validate_name_symbol(*ptr),
                                           "Invalid character '{}' in parent definition in {}:{}",
                                           *ptr, path, n + 1);
                        #else
                            if (!validate_name_symbol(*ptr))
                                throw cfg_detls::CfgException(cfg_detls::str_join(
                                        "Invalid character '", String(1, *ptr), "' in parent definition in ", path, ":",
                                        std::to_string(n + 1).data()));
                        #endif

                        ++ptr;
                    }

                    currentSection->addParent(
                            String(line.substr(start2 - line.cbegin(), ptr - start2)));

                    if (skip_spaces_if_no_endl(ptr, line.cend()))
                        break;

                    #ifdef SCM_ASSERTS
                        SCM_ASSERTS(*ptr == ',', "Missing ',' after parent definition in %s:%z.", path.data(), n + 1);
                    #elif defined SCM_FMT_ASSERTS
                        SCM_FMT_ASSERTS(*ptr == ',', "Missing ',' after parent definition in {}:{}.", path, n + 1);
                    #else
                        if (*ptr != ',')
                            throw cfg_detls::CfgException(cfg_detls::str_join(
                                    "Missing ',' after parent definition in ", path, ":",
                                    std::to_string(n + 1).data()));
                    #endif

                    ++ptr;

                    #ifdef SCM_ASSERTS
                        SCM_ASSERTS(!skip_spaces_if_no_endl(ptr, line.cend()),
                                    "Missing parent parent definition after ',' in %s:%z.",
                                    path.data(), n + 1);
                    #elif defined SCM_FMT_ASSERTS
                        SCM_FMT_ASSERTS(!skip_spaces_if_no_endl(ptr, line.cend()),
                                        "Missing parent parent definition after ',' in {}:{}.",
                                        path, n + 1);
                    #else
                        if (skip_spaces_if_no_endl(ptr, line.cend()))
                            throw cfg_detls::CfgException(cfg_detls::str_join(
                                    "Missing parent parent definition after ',' in ", path, ":",
                                    std::to_string(n + 1).data()));
                    #endif
                }
            }

                //////////////////////////// Preprocessor task //////////////////////////////////////
            else if (*ptr == '#') {
                preprocessorTask(path, n, line.substr(ptr - line.begin()));
            }

                //////////////////////////////// Read variables /////////////////////////////////////
            else {
                auto pair = pairFromLine(path, n, line);

                auto var  = unpackVariable(path, n, pair.second);

                if (currentSection)
                    currentSection->add(String(pair.first), var);
                else {
                    #ifdef SCM_ASSERTS
                        SCM_ASSERTS(!cfgData().section(String(cfg_detls::GLOBAL_NAMESPACE)).isExists(String(pair.first)),
                                    "Duplicate variable '%s' in global namespace in %s:%z",
                                    String(pair.first).data(), path.data(), n + 1);
                    #elif defined SCM_FMT_ASSERTS
                        SCM_FMT_ASSERTS(!cfgData().section(String(cfg_detls::GLOBAL_NAMESPACE)).isExists(String(pair.first)),
                                        "Duplicate variable '{}' in global namespace in {}:{}",
                                        pair.first, path, n + 1);
                    #else
                        if (cfgData().section(String(cfg_detls::GLOBAL_NAMESPACE)).isExists(String(pair.first)))
                            throw cfg_detls::CfgException(cfg_detls::str_join(
                                    "Duplicate variable '", pair.first, "' in global namespace in ", path, ":",
                                    std::to_string(n + 1).data()));
                    #endif

                    cfgData().addValue(String(cfg_detls::GLOBAL_NAMESPACE), String(pair.first), var);
                }
            }
        }
    }


    void processFileTask(StringCref path) {
        auto file  = scm_utils::read_file_to_string(path);
        auto lines = scm_utils::split_view(file, {'\n', '\r', '\0'}, true); // do not delete empty strings

        deleteComments(path, lines);
        parseLinesTask(path, lines);
    }


    void processFileTask() {
        for (auto& path : cfg_detls::cfg_state().getEntries())
            processFileTask(path);
    }

} // namespace cfg_detls

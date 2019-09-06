#pragma once

#include "scm_utils.hpp"
#include "scm_types.hpp"
#include "scm_filesystem.hpp"
#include "scm_aton.hpp"

#ifdef SCM_NAMESPACE
namespace SCM_NAMESPACE {
#endif // SCM_NAMESPACE

    class ConfigManager;

#ifdef SCM_NAMESPACE
}
#endif // SCM_NAMESPACE

#define SIA static inline auto

//////////////////////////////////// Details ///////////////////////////////////

namespace scm_details {
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
    using CfgException  = scm_utils::CfgException;

    static constexpr inline std::string_view GLOBAL_NAMESPACE = "__global";


    // Creation state

    class CfgCreationState {
        friend SCM_NAMESPACE::ConfigManager;
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
        CfgCreationState () {}
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

                    SCM_EXCEPTION(CfgException, !skip_spaces_if_no_endl(ptr, line.cend()),
                                  "Empty key after '$' in ", path, ":", std::to_string(lineNum + 1).data());

                    SCM_EXCEPTION(CfgException, !is_digit(*ptr) && !is_legal_name_symbol(*ptr),
                                  "Starting key with symbol '", String(1, *ptr), "' in ", path, ":",
                                  std::to_string(lineNum + 1).data());

                    auto start = ptr;

                    while(!is_space(*ptr) && *ptr != ':' && ptr != line.cend() && *ptr != '}' && *ptr != ',') {
                        SCM_EXCEPTION(CfgException, validate_name_symbol(*ptr),
                                      "Invalid character '", String(1, *ptr), "' in key after '$' in ", path, ":",
                                      std::to_string(lineNum + 1).data());

                        ++ptr;
                    }

                    auto val   = StrView();
                    auto first = line.substr(start - line.cbegin(), ptr - start);

                    skip_spaces_if_no_endl(ptr, line.cend());

                    ////////// Dereference key
                    if (ptr == line.cend() || *ptr != ':')
                        //////// Read value from global namespaces
                        val = scm_details::cfgData().getValue(String(scm_details::GLOBAL_NAMESPACE), String(first));
                    else if (*ptr == ':'){
                        //////// Read value from section (only no-parents section supported)
                        ++ptr; // skip ':'

                        SCM_EXCEPTION(CfgException, !skip_spaces_if_no_endl(ptr, line.cend()),
                                      "Empty key after ':' in ", path, ":", std::to_string(lineNum + 1).data());

                        SCM_EXCEPTION(CfgException, !is_digit(*ptr) && !is_legal_name_symbol(*ptr),
                                      "Starting key with symbol '", String(1, *ptr), "' in ", path, ":",
                                      std::to_string(lineNum + 1).data());

                        auto start2 = ptr;

                        while(!is_space(*ptr) && *ptr != ':' && ptr != line.cend()) {
                            SCM_EXCEPTION(CfgException, validate_name_symbol(*ptr),
                                          "Invalid character '", String(1, *ptr), "' in key after '$' in ", path, ":",
                                          std::to_string(lineNum + 1).data());

                            ++ptr;
                        }

                        auto second = line.substr(start2 - line.cbegin(), ptr - start2);
                        SCM_EXCEPTION(CfgException, scm_details::cfgData().getSection(String(first)).getParents().empty(),
                                      "Attempt to dereference key '", second, "' from section [", first, "] with parent ",
                                      "in ", path, ":", std::to_string(lineNum + 1).data());

                        val = scm_details::cfgData().getValue(String(first), String(second));
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

        SCM_EXCEPTION(CfgException, !is_digit(*ptr) && !is_legal_name_symbol(*ptr),
                      "Starting key with symbol '", String(1, *ptr), "' in ", path, ":",
                      std::to_string(lineNum + 1).data());

        while(!is_space(*ptr) && *ptr != '=' && ptr != line.cend()) {

            SCM_EXCEPTION(CfgException, validate_name_symbol(*ptr),
                          "Invalid character '", String(1, *ptr), "' in key definition '$' in ", path, ":",
                          std::to_string(lineNum + 1).data());

            ++ptr;
        }

        auto key = line.substr(start - line.cbegin(), ptr - start);


        //////////// Read values (no space deleting inside values)
        SCM_EXCEPTION(CfgException, !skip_spaces_if_no_endl(ptr, line.cend()),
                      "Missing value at key '", key, "' in ", path, ":",
                      std::to_string(lineNum + 1).data());

        SCM_EXCEPTION(CfgException, *ptr == '=', "Missing delimiter '=' at key '", key, "' in ", path, ":",
                      std::to_string(lineNum + 1).data());

        ++ptr;

        auto value = remove_space_bounds_if_exists(line.substr(ptr - line.cbegin(), line.end() - ptr));

        SCM_EXCEPTION(CfgException, !value.empty(), "Missing value at key '", key, "' in ", path, ":",
                      std::to_string(lineNum + 1).data());

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
                    SCM_EXCEPTION(CfgException, validate_symbol(*i),
                                  "Undefined char symbol '", String(1, *i), "' [", std::to_string(unsigned(*i)).data(),
                                  "] in ", path, ":", std::to_string(n + 1).data());

                    if (*i == ';' || (*i == '/' && *(i + 1) == '/')) {
                        line = line.substr(0, i - line.begin());
                        break;
                    }
                }
            }

            SCM_EXCEPTION(CfgException, !onSingleQuotes, "Missing second \' quote in ", path, ":", std::to_string(n + 1).data());
            SCM_EXCEPTION(CfgException, !onDoubleQuotes, "Missing second \" quote in ", path, ":", std::to_string(n + 1).data());
        }
    }



    void preprocessorTask(StringCref path, SizeT lineNum, StrViewCref line) {
        auto ptr = line.cbegin();

        ++ptr; // skip '#'

        SCM_EXCEPTION(CfgException, !skip_spaces_if_no_endl(ptr, line.cend()),
                      "Empty preprocessor directive in ", path, ":", std::to_string(lineNum + 1).data());

        auto start = ptr;

        while (!is_space(*ptr) && ptr != line.cend()) {

            SCM_EXCEPTION(CfgException, is_plain_text(*ptr), "Invalid character in preprocessor directive in ", path, ":",
                          std::to_string(lineNum + 1).data());

            ++ptr;
        }

        auto first    = line.substr(start - line.cbegin(), ptr - start);
        auto backline = line.substr(ptr - line.cbegin(), line.cend() - ptr);

        if (first == "include") {
            auto appendPath = remove_brackets_if_exists(remove_space_bounds_if_exists(backline));

            SCM_EXCEPTION(CfgException, !appendPath.empty(),
                          "Empty path in include directive in ", path, ":",
                          std::to_string(lineNum + 1).data());

            processFileTask(scm_utils::append_path(scm_utils::parent_path(path), String(appendPath)));
        } else {
            SCM_EXCEPTION(CfgException, 0, "Unknown preprocessor directive '#", first, "' in ", path, ":",
                          std::to_string(lineNum + 1).data());
        }
    }



    void parseLinesTask(StringCref path, StrViewVector& lines) {
        using scm_details::Section;
        using scm_details::cfgData;

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

                SCM_EXCEPTION(CfgException, !is_digit(*ptr) && !is_legal_name_symbol(*ptr),
                              "Starting section with symbol '", String(1, *ptr), "' in ", path, ":",
                              std::to_string(n + 1).data());

                while(*ptr != ']' && ptr != line.cend()) {

                    SCM_EXCEPTION(CfgException, validate_name_symbol(*ptr),
                                  "Invalid character '", String(1, *ptr), "' in section definition in ", path, ":",
                                  std::to_string(n + 1).data());

                    ++ptr;
                }

                SCM_EXCEPTION(CfgException, *ptr == ']', "Missing close section bracket in ", path, ":",
                              std::to_string(n + 1).data());

                currentSection = &cfgData().addSection(path, n, String(line.substr(start - line.cbegin(), ptr - start)));

                ++ptr; // skip ']'

                if (skip_spaces_if_no_endl(ptr, line.cend()))
                    continue;


                /////////////// Read parents

                SCM_EXCEPTION(CfgException, *ptr == ':',
                              "Unexpected symbol '", String(1, *ptr), "' after section [", currentSection->name(),
                              "] definition in ", path, ":", std::to_string(n + 1).data());

                ++ptr; // skip ':'

                SCM_EXCEPTION(CfgException, !skip_spaces_if_no_endl(ptr, line.cend()),
                              "Missing parents sections after ':' in ", path, ":",
                              std::to_string(n + 1).data());

                while(ptr != line.end()) {
                    auto start2 = ptr;

                    SCM_EXCEPTION(CfgException, !is_digit(*ptr) && !is_legal_name_symbol(*ptr),
                                  "Starting parent definition with symbol '", String(1, *ptr), "' in ", path, ":",
                                  std::to_string(n + 1).data());

                    while(!is_space(*ptr) && ptr != line.cend() && *ptr != ',') {
                        SCM_EXCEPTION(CfgException, validate_name_symbol(*ptr),
                                      "Invalid character '", String(1, *ptr), "' in parent definition in ", path, ":",
                                      std::to_string(n + 1).data());

                        ++ptr;
                    }

                    currentSection->addParent(
                            String(line.substr(start2 - line.cbegin(), ptr - start2)));

                    if (skip_spaces_if_no_endl(ptr, line.cend()))
                        break;

                    SCM_EXCEPTION(CfgException, *ptr == ',', "Missing ',' after parent definition in ", path, ":",
                                  std::to_string(n + 1).data());

                    ++ptr;

                    SCM_EXCEPTION(CfgException, !skip_spaces_if_no_endl(ptr, line.cend()),
                                  "Missing parent parent definition after ',' in ", path, ":",
                                  std::to_string(n + 1).data());
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
                    SCM_EXCEPTION(CfgException,
                                  !cfgData().section(String(scm_details::GLOBAL_NAMESPACE)).isExists(String(pair.first)),
                                  "Duplicate variable '", pair.first, "' in global namespace in ", path, ":",
                                  std::to_string(n + 1).data());

                    cfgData().addValue(String(scm_details::GLOBAL_NAMESPACE), String(pair.first), var);
                }
            }
        }
    }

    static auto unpack
            (StrViewCref name, StrViewCref section, StrViewCref str, SizeT required = 0) -> StrViewVector
    {
        StrViewVector vec;

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
                                    SCM_EXCEPTION(CfgException, *(ptr + 1) != '}',
                                                  "Redundant '}' at key '", name, "' in section [", section, "].");

                                    SCM_EXCEPTION(CfgException, *(ptr + 1) == ',',
                                                  "Missing ',' at key '", name, "' in section [", section, "].");
                                }

                                break;
                            }
                        }
                    }
                }

                SCM_EXCEPTION(CfgException, entryLevel == 0,
                              "Missing close '}' at key '", name, "' in section [", section, "].");

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

                        SCM_EXCEPTION(CfgException, scm_details::validate_keyval(*ptr),
                                      "Undefined char symbol '", String(1, *ptr), "' at key '", name,
                                      "' in section [", section, "].");
                    }
                }

                vec.emplace_back(str.substr(start - str.cbegin(), ptr - start));

                if (ptr != str.cend())
                    ++ptr;
            }
        }

        if (vec.size() == 1 && vec[0].size() > 1 && vec[0].front() == '{' && vec[0].back() == '}')
            return unpack(name, section, vec[0].substr(1, vec[0].size() - 2), required);

        SCM_EXCEPTION(CfgException, required == 0 || required == vec.size(),
                      "Wrong number of values at key '", name, "' in section [", section, "]. Provided ",
                      std::to_string(vec.size()).data(), ", required ", std::to_string(required).data(), ".");

        return std::move(vec);
    }

    // Numbers
    template <typename T>
    SIA superCast(StrViewCref str, StrViewCref, StrViewCref)
    -> std::enable_if_t<scm_utils::numbers<T>, T> {
        return scm_utils::aton<T>(str);
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
            SCM_EXCEPTION(CfgException, 0, "Unknown bool value '", str, "' at key '",
                          name.data(), "' in section [", section.data(), "].");
        return false; // !?
    }

    template <typename A, typename T = std::remove_reference_t<decltype(std::declval<A>()[0])>, SizeT _Size = sizeof(A)/sizeof(T)>
    SIA superCast(StrViewCref str, StrViewCref name, StrViewCref section)
    -> std::enable_if_t<scm_utils::any_of<A, ScmArray<T, _Size>, std::array<T, _Size>>, A>;

    template <typename A, typename T = std::remove_reference_t<decltype(std::declval<A>()[0])>>
    SIA superCast(StrViewCref str, StrViewCref name, StrViewCref section)
    -> std::enable_if_t<scm_utils::any_of<A, ScmVector<T>, std::vector<T>>, A>;

    //
    template <typename... Ts, SizeT... _Idx>
    SIA readTupleImpl(StrViewVector& vec, StrViewCref name, StrViewCref section, std::index_sequence<_Idx...>) {
        return std::make_tuple(superCast<Ts>(vec[_Idx], name, section)...);
    }

    template <typename T, SizeT... _Idx>
    SIA readArrayImpl(StrViewVector& vec, StrViewCref name, StrViewCref section, std::index_sequence<_Idx...>) {
        auto arr = ScmArray<T, sizeof...(_Idx)>{};

        ((arr[_Idx] = superCast<T>(vec[_Idx], name, section)) , ...);

        return arr;
    }

    template <typename T>
    SIA readVectorImpl(StrViewVector& vec, StrViewCref name, StrViewCref section) {
        auto res = ScmVector<T>{};

        for (auto& s : vec)
            res.push_back(superCast<T>(s, name, section));

        return res;
    }

    // Array
    template <typename A, typename T, SizeT _Size>
    SIA superCast(StrViewCref str, StrViewCref name, StrViewCref section)
    -> std::enable_if_t<scm_utils::any_of<A, ScmArray<T, _Size>, std::array<T, _Size>>, A> {
        auto vec = unpack(name, section, str, _Size);
        return readArrayImpl<T>(vec, name, section, std::make_index_sequence<_Size>());
    }

    // Vector
    template <typename A, typename T>
    SIA superCast(StrViewCref str, StrViewCref name, StrViewCref section)
    -> std::enable_if_t<scm_utils::any_of<A, ScmVector<T>, std::vector<T>>, A> {
        auto vec = unpack(name, section, str);
        return readVectorImpl<T>(vec, name, section);
    }


    void processFileTask(StringCref path) {
        auto file  = scm_utils::read_file_to_string(path);
        auto lines = scm_utils::split_view(file, {'\n', '\r', '\0'}, true); // do not delete empty strings

        deleteComments(path, lines);
        parseLinesTask(path, lines);
    }


    void processFileTask() {
        for (auto& path : scm_details::cfg_state().getEntries())
            processFileTask(path);
    }

} // namespace scm_details

#undef SIA
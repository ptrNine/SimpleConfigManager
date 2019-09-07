#pragma once
namespace scm_details {
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

    auto unpackVariable (StrViewCref path, SizeT lineNum, StrViewCref line) -> String {
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
                        val = cfg_data().getValue(String(GLOBAL_NAMESPACE), String(first));
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
                        SCM_EXCEPTION(CfgException, cfg_data().getSection(String(first)).getParents().empty(),
                                      "Attempt to dereference key '", second, "' from section [", first, "] with parent ",
                                      "in ", path, ":", std::to_string(lineNum + 1).data());

                        val = cfg_data().getValue(String(first), String(second));
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


    auto pairFromLine(StrViewCref path, SizeT lineNum, StrViewCref line) -> StrViewPair {
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


    void deleteComments(StrViewCref path, StrViewVector& lines) {
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

    void processFileTask(StrViewCref path);

    void preprocessorTask(StrViewCref path, SizeT lineNum, StrViewCref line) {
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

            processFileTask(SCM_NAMESPACE::append_path(SCM_NAMESPACE::parent_path(path), String(appendPath)));
        } else {
            SCM_EXCEPTION(CfgException, 0, "Unknown preprocessor directive '#", first, "' in ", path, ":",
                          std::to_string(lineNum + 1).data());
        }
    }


    void parseLinesTask(StrViewCref path, StrViewVector& lines) {
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

                currentSection = &cfg_data().addSection(String(path), n, String(line.substr(start - line.cbegin(), ptr - start)));

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
                                  !cfg_data().section(String(GLOBAL_NAMESPACE)).isExists(String(pair.first)),
                                  "Duplicate variable '", pair.first, "' in global namespace in ", path, ":",
                                  std::to_string(n + 1).data());

                    cfg_data().addValue(String(GLOBAL_NAMESPACE), String(pair.first), var);
                }
            }
        }
    }


    auto unpack(StrViewCref name, StrViewCref section, StrViewCref str, SizeT required) -> StrViewVector
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

                        SCM_EXCEPTION(CfgException, validate_keyval(*ptr),
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

    void processFileTask(StrViewCref path) {
        auto file  = SCM_NAMESPACE::read_file_to_string(path);
        auto lines = SCM_NAMESPACE::split_view(file, {'\n', '\r', '\0'}, true); // do not delete empty strings

        deleteComments(path, lines);
        parseLinesTask(path, lines);
    }

    void parse(StrViewCref path) {
        processFileTask(path);
        cfg_data().reloadParents();
    }

} // namespace scm_details
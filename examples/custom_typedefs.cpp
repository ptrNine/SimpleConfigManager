#include <map>
#include <string>

// Use std::map as ScmMap (instead std::unordered_map)
#define SCM_TYPE_MAP std::map


// Use MyString as ScmString

class MyString : public std::string {
public:
    using super = std::string;

    MyString(): super() {}
    MyString(const char* str):             super(str)  {}
    MyString(const std::string_view& str): super(str)  {}
    MyString(const std::string& str):      super(str)  {}
    MyString(std::size_t n, char c):       super(n, c) {}

    operator std::string_view() const { return this->c_str(); }

    int to_number() { return std::stoi(*this); }
};

#define SCM_TYPE_STRING MyString


#include <scm/scm.hpp>


int main() {
    scm::parse(scm::fs::default_cfg_path());

    auto name = scm::read<ScmString>("money", "player");

    std::cout << name.to_number() << std::endl;
    
    return 0;
}
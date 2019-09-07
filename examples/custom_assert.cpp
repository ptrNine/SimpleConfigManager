#include <string>
#include <iostream>

// Don't use format and no variable arguments in this function to simplify example :)
void custom_assert(bool condition, const char* file, int line, const std::string& format, const std::string& what) {
    if (!condition) {
        std::cerr << "[Fatal error]: " << std::endl;
        std::cerr << "What: " << what << std::endl;
        std::cerr << "Source: " << file << ":" << line << std::endl;
    }
}

#define CUSTOM_ASSERT(COND, FMT, ...) \
    custom_assert((COND), __FILE__, __LINE__, FMT, __VA_ARGS__)

#define SCM_ASSERTS CUSTOM_ASSERT

// Use SCM_FMT_ASSERTS if you used fmt based asserts

#include <scm/scm.hpp>

int main() {
    scm::parse(scm::fs::default_cfg_path());

    // Crash with own assert
    auto weapon = scm::read<std::string>("weapon", "player");

    return 0;
}
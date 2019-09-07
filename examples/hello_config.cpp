#include <iostream>
#include <scm/scm.hpp>

int main() {
    scm::parse(scm::fs::default_cfg_path());

    std::cout << scm::read<std::string>("key", "hello_config") << std::endl;
    
    return 0;
}
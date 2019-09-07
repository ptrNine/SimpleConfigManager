#include <iostream>

#define SCM_NAMESPACE custom_namespace

#include <scm/scm.hpp>

int main() {
    custom_namespace::parse(custom_namespace::fs::default_cfg_path());
    
    std::cout << custom_namespace::read<int>("health", "other_player") << std::endl;
    
    return 0;
}
#include "test2.hpp"

#include <iostream>
#include "cfg.hpp"

Test2::Test2() {
    std::cout << cfg::read<std::string>("test2", "noinline") << std::endl;
}
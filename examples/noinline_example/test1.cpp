#include "test1.hpp"

#include <iostream>
#include "cfg.hpp"

Test1::Test1() {
    std::cout << cfg::read<std::string>("test1", "noinline");
}
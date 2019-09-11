#include "cfg.hpp"

#include "test1.hpp"
#include "test2.hpp"

int main() {
    cfg::parse(cfg::fs::default_cfg_path());

    Test1();
    Test2();

    return 0;
}
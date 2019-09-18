# SimpleConfigManager
A parser for ini-like configs
# Introduction
SCM (SimpleConfigManager) is:
* small and can be used as header-only (except filesystem module)
* highly-customizable, allow defining custom config types, allow using you own 
  asserts, renaming namespaces, etc.
* easy-to-use, has some handy features, allowed inheritance in ini-section 
  and interpolation (like python configparser)
# Examples
SCM use templates for type recognition. For example, value `vector` from 
file `/home/user/test.cfg` can be read by this way:  
Ini file:
```ini
[section]
vector = 1, 2, 3, 4, 5
```
C++:
```c++
#include <iostream>
#include <vector>
#include <scm/scm.hpp>

int main() {
    scm::parse("/home/user/test.cfg");
    
    // Read 1, 2, 3, 4, 5
    auto vec = scm::read<std::vector<int>>("vector", "section");
    
    return 0;
}
```
SCM can read values of any complexity (if it ever comes in handly):  
Ini-file:
```ini
[sect]
val = "string", true, 14.5E-2, {{1,2},{3,4},{5,6}} 
```
C++:
```c++
int main() {
...
    auto [a, b, c, d] = scm::read<
            std::string, 
            bool, 
            float, 
            std::vector<std::pair<int, int>>
   >("val", "sect");
...
}
```
Some special macros in class constructor make config reading much easier:  
Ini-file:
```ini
[player]
name = "player"
health = 100
money = 1000
spawn_pos = 10, 24.3
```
C++:
```c++
class Player {
public:
    Player(const std::string_view& section) {
        // define current section for macroses;
        auto scm_current_sect = section;
        
        // Easily read all members
        SCM_MSET_CUR(name);
        SCM_MSET_CUR(health);
        SCM_MSET_CUR(money);
        SCM_MSET_CUR(spawn_pos);
    }
    
private:
    std::string _name;
    float _health;
    unsigned _money;
    std::pair<float, float> _spawn_pos;
};
```
#### Interpolation of values:
```ini
[dirs]
data = /usr/share/game_dir/
textures = $data textures/      ; textures = /usr/share/game_dir/textures/

[tilemap1]
texture = $dirs:textures tilemap1.png   ; texture = /usr/share/game_dir/textures/tilemap1.png

```
#### Inheritance:
```ini
[sect1]
one = 1
two = "two"

[sect2] : sect1    ; one and two inherits from sect1
one = 2            ; redefining one
```
#### Multi-inheritance:
```ini
[sect1]
val = 1

[sect2]
val = 2

[sect3] : sect1, sect2
; val = 1 because sect1 has higher priority

```
Check [this](https://github.com/ptrNine/SimpleConfigManager/tree/master/examples) for all examples.

# Building
SCM can be build with CMake or can be used as header-only library (except the filesystem module).
C++17 is required (GCC with version >= 8.3.0 (MinGW) or clang-7 or later).  

For use as header-only library, just copy 'scm' dir. For using filesystem module you 
should add `scm/scm_filesystem.cpp` to your executable or library. 

## Building with CMake

### On Linux:

```bash
# Clone SCM repo
git clone https://github.com/ptrNine/SimpleConfigManager

cd SimpleConfigManager

# Checkout to latest release
git checkout v0.1.0 -b release

# Run cmake
# Set -DSCM_BUILD_TESTS=ON for building tests
# Set -DSCM_BUILD_EXAMPLES=ON for building examples
cmake . -Bbuild -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="your/install/path/"

# Build and install
# Install in "/usr/local/" (by default) require root rights
make -C build install

```
### On Windows:

```bat
:: Clone SCM repo
git clone https://github.com/ptrNine/SimpleConfigManager

cd SimpleConfigManager

:: Checkout to latest release
git checkout v0.1.0 -b release

:: Run cmake
:: Set -DSCM_BUILD_TESTS=ON for building tests
:: Set -DSCM_BUILD_EXAMPLES=ON for building examples
cmake . -G "MinGW Makefiles" -Bbuild -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="your/install/path/"

:: Build and install
mingw32-make -C build install

```
### Usage in CMake-based project
```cmake
# Specify path for SCMConfig.cmake if custom install path was used
# set(SCM_DIR "PATH/TO/SCM/CONFIG")

# Find SCM
find_package(SCM REQUIRED)

# Set include and link directories via use file
include(${SCM_USE_FILE})

# SCM_STATIC_LIBRARIES can also be used
add_executable(your_exec ${SCM_LIBRARIES})

```
## SCM Ini format
See [that](https://github.com/ptrNine/SimpleConfigManager/tree/master/SCM_ini_readme.md) for information.

## WIP!

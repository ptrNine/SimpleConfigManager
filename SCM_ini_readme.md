## SCM Ini format
Section and key names can't be started with a digit and must be combination of any of of the 
following characters:
```ini
; abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789.@/\-

[123section]      ; wrong
[section@foo.bar] ; valid 
```

Strings with other symbols (spaces, for example) must be quoted with `'` or `"`:
```ini
key1 = "It's a string"   ; valid
key2 = 'It's a string'   ; invalid, missing ' symbol
key3 = 'Its a string'    ; valid
key4 = Its a string      ; valid, but key4 become Itsastring

```

Duplicate sections is prohibited (except the `__global` section):
```ini
[section1]
...

[section2]
...

[section1]  ; Invalid, duplicate section
```

Multiple values in multiple values must be bracketed with `{}`:
```ini
vectors = {1, 2}, {2, 4}, {{6, 9}, {7, 3}}
;          v  v    v  v     v  v    v  v      <- integers
;          val     val      val     val       <- multiple values
;                            big_value        <- multiple of multiples
;                very big value               <- multiple of multiple of ...

```

Comments start with `;` or `//`:
```ini
; it's a comment
// it's a comment
```

File includes same as in C:
```ini
#include "path/to/another.cfg"
#include 'path/to/another.cfg'
#include path/to/another.cfg
```

Keys without section definition located in `__global` section.
You can read global values in C++ without setting "__global"
```ini
; No one section define in this file. It's the global section
global_value = "value!"

[some_sect]
...

; Explicit setting the global section
[__global]
another_global_value = "another value!"
```
C++:
```c++
auto value1 = scm::read<std::string>("another_global_value");     // valid
auto value2 = scm::read<std::string>("global_value", "__global"); // also valid, but redundant
```
...WIP!
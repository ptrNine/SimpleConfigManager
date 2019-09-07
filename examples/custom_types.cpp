#include <iostream>
#include <vector>

// For supercast
#include <scm/scm_utils.hpp>

// Implement simple 2D vector
template <typename T>
class Vector2d {
public:
    using Type = T;
    Vector2d(T x, T y): _x(x), _y(y) {}

    auto x() const { return _x; }
    auto y() const { return _y; }

    void x(const T& ix) { _x = ix; }
    void y(const T& iy) { _y = iy; }

    friend std::ostream& operator<<(std::ostream& os, const Vector2d& vec) {
        os << "{ " << vec.x() << ", " << vec.y() << " }";
        return os;
    }

private:
    T _x, _y;
};


// Provide "supercast" for our "Vector2d" type

namespace scm_details {
    template<typename V, typename T = typename V::Type>
    auto SCM_SUPERCAST() -> std::enable_if_t<std::is_same_v<V, Vector2d<T>>, V> {
        auto args = SCM_UNPACK(2); // Unpack to 2 strings

        // Return our vector and call 'superCast' with unpacked args for further transformations
        // If you want Vector2d with numbers only, you can use 'atoi' instead superCast
        return Vector2d {
            superCast<T>(args[0], SCM_SUPERCAST_ARGS),
            superCast<T>(args[1], SCM_SUPERCAST_ARGS)
        };
    }
}

// Now including scm
#include <scm/scm.hpp>


int main() {
    scm::parse(scm::fs::default_cfg_path());

    std::cout << scm::read<Vector2d<int>>("value1", "custom_types") << std::endl;
    std::cout << scm::read<Vector2d<int>>("value2", "custom_types") << std::endl;

    // This also work!
    std::cout << scm::read<Vector2d<Vector2d<int>>>("value3", "custom_types") << std::endl;

    auto vec = scm::read<std::vector<Vector2d<int>>>("value4", "custom_types");
    for (auto& e : vec)
        std::cout << e << "   ";
    std::cout << std::endl;


    // And this work too
    auto [a, b, c, d] =
            scm::read<
                Vector2d<int>,
                Vector2d<float>,
                Vector2d<bool>,
                Vector2d<std::string>
            > ("value5", "custom_types");

    std::cout << a << std::endl;
    std::cout << b << std::endl;
    std::cout << c << std::endl;
    std::cout << d << std::endl;

    return 0;
}
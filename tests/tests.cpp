#include <gtest/gtest.h>

#define SCM_NAMESPACE cfg

#include <scm/scm_utils.hpp>
#include <scm/scm_aton.hpp>


template <typename T>
class Vector2d {
    T _x, _y;

public:
    using Type = T;
    Vector2d(T ix, T iy): _x(ix), _y(iy) {}

    auto& x() const { return _x; }
    auto& y() const { return _y; }
    auto& x() { return _x; }
    auto& y() { return _y; }
};

namespace scm_details {
    template<typename A, typename T = typename A::Type>
    auto SCM_SUPERCAST() -> std::enable_if_t<cfg::any_of<A, Vector2d<T>>, A> {
        auto vec = SCM_UNPACK(2);
        return Vector2d(cfg::aton<T>(vec[0]), cfg::aton<T>(vec[1]));
    }
}


#include <scm/scm.hpp>

using U32 = uint32_t;
using S32 = int32_t;
using Float32 = float;
using Float64 = double;
using String = ScmString;

#define TEST_SECTION(SECT_NAME) \
ASSERT_EQ      (cfg::read<U32>     ("one", SECT_NAME),      100); \
ASSERT_EQ      (cfg::read<S32>     ("two", SECT_NAME),      -101); \
ASSERT_EQ      (cfg::read<U32>     ("three", SECT_NAME),    102); \
ASSERT_FLOAT_EQ(cfg::read<Float32> ("four", SECT_NAME),     103.3f); \
ASSERT_FLOAT_EQ(cfg::read<Float64> ("five", SECT_NAME),     -104.4f); \
ASSERT_FLOAT_EQ(cfg::read<Float32> ("six", SECT_NAME),      10.55E1f); \
ASSERT_FLOAT_EQ(cfg::read<Float64> ("seven", SECT_NAME),    -10.66E1); \
ASSERT_FLOAT_EQ(cfg::read<Float64> ("eight", SECT_NAME),    1077E-1); \
ASSERT_FLOAT_EQ(cfg::read<Float32> ("nine", SECT_NAME),     -1088E-1); \
ASSERT_EQ      (cfg::read<String>  ("ten", SECT_NAME),      " 'test string?><%&${}' "); \
ASSERT_EQ      (cfg::read<String>  ("eleven", SECT_NAME),   " \"test string?><%&${}\" "); \
ASSERT_EQ      (cfg::read<String>  ("twelve", SECT_NAME),   "teststring"); \
ASSERT_EQ      (cfg::read<bool>    ("thirteen", SECT_NAME), true); \
ASSERT_EQ      (cfg::read<bool>    ("fourteen", SECT_NAME), false); \
ASSERT_EQ      (cfg::read<bool>    ("fifteen", SECT_NAME),  true); \
ASSERT_EQ      (cfg::read<bool>    ("sixteen", SECT_NAME),  false)

TEST(ConfigTests, TestSection) {
    auto path = cfg::append_path(cfg::fs::current_path(), String("test.cfg"));

    cfg::reload(path);

    // Global section
    ASSERT_EQ      (cfg::read<U32>     ("g_one"),      100);
    ASSERT_EQ      (cfg::read<S32>     ("g_two"),      -101);
    ASSERT_EQ      (cfg::read<U32>     ("g_three"),    102);
    ASSERT_FLOAT_EQ(cfg::read<Float32> ("g_four"),     103.3f);
    ASSERT_FLOAT_EQ(cfg::read<Float64> ("g_five"),     -104.4f);
    ASSERT_FLOAT_EQ(cfg::read<Float32> ("g_six"),      10.55E1f);
    ASSERT_FLOAT_EQ(cfg::read<Float64> ("g_seven"),    -10.66E1);
    ASSERT_FLOAT_EQ(cfg::read<Float64> ("g_eight"),    1077E-1);
    ASSERT_FLOAT_EQ(cfg::read<Float32> ("g_nine"),     -1088E-1);
    ASSERT_EQ      (cfg::read<String>  ("g_ten"),      " 'test string?><%&${}' ");
    ASSERT_EQ      (cfg::read<String>  ("g_eleven"),   " \"test string?><%&${}\" ");
    ASSERT_EQ      (cfg::read<String>  ("g_twelve"),   "teststring");
    ASSERT_EQ      (cfg::read<bool>    ("g_thirteen"), true);
    ASSERT_EQ      (cfg::read<bool>    ("g_fourteen"), false);
    ASSERT_EQ      (cfg::read<bool>    ("g_fifteen"),  true);
    ASSERT_EQ      (cfg::read<bool>    ("g_sixteen"),  false);

    TEST_SECTION("test_section_single1");
    TEST_SECTION("test_section_single2");
    TEST_SECTION("test_section_single3");
    TEST_SECTION("test_section_single4");
    TEST_SECTION("test_section_single5");

    auto [i, f, b, s] = cfg::read<S32, float, bool, String>("tuple", "test_section_multi1");


    ASSERT_EQ(i, 10);
    ASSERT_EQ(f, 1.1f);
    ASSERT_EQ(b, true);
    ASSERT_EQ(s, "sample text");

    auto intarray   = std::array{10, 20, 30, 40, 50};
    auto intarray_r = cfg::read<ScmArray<S32, 5>>("intarray", "test_section_multi1");
    ASSERT_EQ(intarray, intarray_r);

    auto strarray   = ScmArray<String, 3>{"kek", "heh heh", " 333 444"};
    auto strarray_r = cfg::read<ScmArray<String, 3>>("stringarray", "test_section_multi1");
    ASSERT_EQ(strarray, strarray_r);

    auto intdblarray = cfg::read<ScmArray<ScmArray<S32, 3>, 3>>("intdblarray", "test_section_multi1");
    auto test1 = ScmArray<ScmArray<S32, 3>, 3>{
            ScmArray<S32, 3>{ 1, 2, 3 },
            ScmArray<S32, 3>{ 5, 6, 7 },
            ScmArray<S32, 3>{ 6, 7, 8 }
    };
    ASSERT_EQ(intdblarray, test1);

    auto intdblvector = cfg::read<ScmVector<ScmVector<S32>>>("intdblvector", "test_section_multi1");
    auto test2 = ScmVector<ScmVector<S32>>{
            ScmVector<S32>{ 1, 2, 3, 4 },
            ScmVector<S32>{ 1, 2 },
            ScmVector<S32>{ 2, 3, 4, 5, 6 }
    };
    ASSERT_EQ(intdblvector, test2);

    auto inttrplvector = cfg::read<ScmVector<ScmVector<ScmVector<S32>>>>("inttrplvector", "test_section_multi1");
    auto test3 = ScmVector<ScmVector<ScmVector<S32>>>{
            ScmVector<ScmVector<S32>>{
                    ScmVector<S32>{1, 2},
                    ScmVector<S32>{3, 2}
            },
            ScmVector<ScmVector<S32>>{
                    ScmVector<S32>{1, 4, 6},
                    ScmVector<S32>{4, 5, 6, 7}
            },
            ScmVector<ScmVector<S32>>{
                    ScmVector<S32>{1}
            }
    };
    ASSERT_EQ(inttrplvector, test3);

    auto vec2d = cfg::read<Vector2d<U32>>("intvec2", "test_section_multi1");
    ASSERT_EQ(vec2d.x(), 10);
    ASSERT_EQ(vec2d.y(), 20);
}


int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
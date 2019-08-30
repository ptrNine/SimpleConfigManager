#include <gtest/gtest.h>
#include "../base/configs.hpp"
#include "../base/ftl/array.hpp"

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
    namespace cfg = base::cfg;
    using String  = ftl::String;

    auto test_config = base::fs::current_path().parent_path() / "test.cfg";
    cfg::resetCfgEntries();
    cfg::addCfgEntry(test_config);


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

    ASSERT_EQ      (cfg::read<ftl::Vector2u32>("intvec2", "test_section_multi1"), ftl::Vector2u32(10, 20));
    ASSERT_EQ      (cfg::read<ftl::Vector3u32>("intvec3", "test_section_multi1"), ftl::Vector3u32(10, 20, 30));
    ASSERT_EQ      (cfg::read<ftl::Vector2f32>("fltvec2", "test_section_multi1"), ftl::Vector2f32(1.1, 2.2));
    ASSERT_EQ      (cfg::read<ftl::Vector3f32>("fltvec3", "test_section_multi1"), ftl::Vector3f32(1.1, 2.2, 3.3));

    auto [i, f, b, s, v] = cfg::read<S32, float, bool, String, ftl::Vector2f64>("tuple", "test_section_multi1");

    ASSERT_EQ(i, 10);
    ASSERT_EQ(f, 1.1f);
    ASSERT_EQ(b, true);
    ASSERT_EQ(s, "sample text");
    ASSERT_EQ(v, ftl::Vector2f64(2.2, 3.3));

    auto intarray   = ftl::Array{10, 20, 30, 40, 50};
    auto intarray_r = cfg::read<ftl::Array<S32, 5>>("intarray", "test_section_multi1");
    ASSERT_EQ(intarray, intarray_r);

    auto strarray   = ftl::Array<String, 3>{"kek", "heh heh", " 333 444"};
    auto strarray_r = cfg::read<ftl::Array<String, 3>>("stringarray", "test_section_multi1");
    ASSERT_EQ(strarray, strarray_r);

    auto intdblarray = cfg::read<ftl::Array<ftl::Array<S32, 3>, 3>>("intdblarray", "test_section_multi1");
    ASSERT_EQ(intdblarray.to_string(), "{ { 1, 2, 3 }, { 5, 6, 7 }, { 6, 7, 8 } }");

    auto intdblvector = cfg::read<ftl::Vector<ftl::Vector<S32>>>("intdblvector", "test_section_multi1");
    ASSERT_EQ(intdblvector.to_string(), "{ { 1, 2, 3, 4 }, { 1, 2 }, { 2, 3, 4, 5, 6 } }");

    // Yeah
    auto inttrplvector = cfg::read<ftl::Vector<ftl::Vector<ftl::Vector<S32>>>>("inttrplvector", "test_section_multi1");
    ASSERT_EQ(inttrplvector.to_string(), "{ { { 1, 2 }, { 3, 2 } }, { { 1, 4, 6 }, { 4, 5, 6, 7 } }, { { 1 } } }");
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
#include "../src/engine.cpp"
#include <gtest/gtest.h>
 
TEST(SecretFileNames, CheckRegexes) { 
    Engine e;
    e.Init();
    ASSERT_EQ(0, 0);
    ASSERT_TRUE(e.search_for_content_match("prod.pfx"));
}
 
 
int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

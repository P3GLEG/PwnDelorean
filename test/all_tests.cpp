#include <gtest/gtest.h>
#include "../src/util.h"

int main(int argc, char **argv){
    static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::init(plog::debug, &consoleAppender);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

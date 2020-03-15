#include "GearboyCore.h"
#include <iostream>
#include <stdio.h>
#include <gtest/gtest.h>

using namespace std;

GearboyCore gc;
const char* szPath;

TEST(Save_test, Save_state){
    printf("Test Case 1\n");
    EXPECT_EQ(1, gc.SaveState(0, 613936576));
    printf("\nTest Case 1 complete !\n");

}

int main(int argc, char **argv){
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

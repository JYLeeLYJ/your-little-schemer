#include <gtest/gtest.h>

#include "runtime.h"

using namespace lispy;

TEST(test_lispy , test_atom_law){
    EXPECT_EQ(Runtime::eval("(atom? 'atom)") , "true");
    EXPECT_EQ(Runtime::eval("(atom? 'turkey)") , "true");
    EXPECT_EQ(Runtime::eval("(atom? 1492)") , "true");
    EXPECT_EQ(Runtime::eval("(atom? 'u)") , "true");
    EXPECT_EQ(Runtime::eval("(atom? '())") , "false");
}

TEST(test_lispy , test_car_law){

}
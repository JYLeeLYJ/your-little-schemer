#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>
#include <experimental/socket>

#include "parser/v1/types.hpp"
#include "parser/v1/parser.hpp"

using namespace std::literals;

TEST(test_v1 , test_types){
    cvector<int> v{1,2,3,4};
    EXPECT_EQ(v.size() , 4);
    v.push_back(5);
    for(auto i = 1 ; auto x : v){
        EXPECT_EQ(x , i);
        ++i;
    }

    EXPECT_EQ(v.capacity() , 6);
    auto v2 = v;

    EXPECT_EQ(v2.size() , v.size());
    EXPECT_EQ(v2.capacity() , v.capacity());
    
    for(auto i = 1 ; auto x : v2){
        EXPECT_EQ(x , i);
        ++i;
    }

}

TEST(test_v1 , test_char_parser){
    auto res1 = item("");
    EXPECT_FALSE(res1);

    auto res2 = item("11");
    EXPECT_TRUE(res2);
    EXPECT_EQ(res2->first , '1');

    auto res3 ='1'_char ("111");
    EXPECT_TRUE(res3);
    EXPECT_EQ(res3->first , '1');
    EXPECT_EQ(res3->second , "11");

    auto res4 = "233"_str ("2333333");
    EXPECT_TRUE(res4);
    EXPECT_EQ(res4->first , "233");
    EXPECT_EQ(res4->second , "3333");

    auto res5 = alphanum("544");
    EXPECT_TRUE(res5);
    EXPECT_EQ(res5->first , '5');
}

TEST(test_v1 , test_combinator){
    
}


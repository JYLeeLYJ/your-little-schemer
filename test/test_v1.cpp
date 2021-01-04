#include <memory>
#include <string>
#include <vector>

#include "parser/v1/types.hpp"
#include "parser/v1/parser.hpp"

#include <gtest/gtest.h>

using namespace std::literals;
using namespace pscpp;

constexpr auto test_const(){
    int n = 0;
    cvector v{1,2,3,4};
    n = v.size();
    return n;
}

constexpr auto test_cv(){
    cvector v{1,2};
    v.push_back(3);
    v.push_back(4);
    return v;
}

constexpr auto test_cv_copy(){
    cvector v{1,2};
    cvector v2{1,2,3,4,5};
    v = v2;

    return v.capacity();
}

TEST(test_v1 , test_types){

    static_assert(test_const() == 4);
    static_assert(test_cv().size() == 4);
    static_assert(test_cv_copy() == 7);

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

    auto res6 = spaces("   ");
    EXPECT_TRUE(res6);
    EXPECT_EQ(res6->first , none_t{});
}

TEST(test_v1 , test_combinator){
    
    //result , bind , lift , plus

    // >> << >>= |

    auto res = ('['_char >> digit << ']'_char) ("[1]");
    EXPECT_TRUE(res);
    EXPECT_EQ(res->first , '1');
}


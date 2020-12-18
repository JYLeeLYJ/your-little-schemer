#include "parser/parsec_experimental.h"

#include <gtest/gtest.h>
#include <type_traits>
#include <vector>

using namespace std::literals;

namespace static_test{

using is_optional = hkt<std::optional>;

template<class T>
using option = std::optional<T>;

static_assert(hkt<option>::match_kind<std::optional<int>>{});
static_assert(is_optional::match_kind<std::optional<int>>{});
static_assert(is_optional::match_kind<int>{} == false);

// using type = decltype(std::function{std::plus<int>{}});
auto  p = std::function{std::plus<int>{}};

static_assert(std::is_same_v<decltype(p)::result_type , int>);

product_type<int ,int> t1;

using tuple_int2 = std::tuple<int,int>;

static_assert(std::is_same_v<product_type<int,int,int> ,decltype(product(t1 , 0))>);
static_assert(std::is_same_v<product_type<int,int> ,decltype(product(0,0))>);
static_assert(std::is_same_v<product_type<int,int,int> ,decltype(product(0,t1))>);  
static_assert(std::is_same_v<product_type<int,int,int,int> , decltype(product(t1,t1))>);   
static_assert(std::is_same_v<product_type<tuple_int2 , int,int> , decltype(product(tuple_int2{} , t1))>);

static_assert(is_parser_result<parser_result<int>>());
static_assert(!is_parser_result<int>());
static_assert(parser_traits<decltype(result(1))>{});
static_assert(std::is_same_v<parser_traits<decltype(result(1))>::type , int>);

};

int test(int x, int y, int z) {
    return x + y + z;
}

TEST(test_functional , test_curry){
    auto f = make_curry(test)(1);
    auto g = f(2);
    auto result = g(3);
    auto result1 = make_curry(test)(1)(2)(3);

    EXPECT_EQ(result , 6);
    EXPECT_EQ(result1, 6);
}

TEST(test_functional , test_functor){
    auto map_int = [](char a){ return int{a};} ;
    auto get_int = fmap(map_int, item );

    auto res = get_int("a");
    EXPECT_TRUE(res);
    EXPECT_EQ(res->first , int{'a'});
    EXPECT_EQ(res->second , ""sv);

}

template<class T>
T inc(T x){return ++x;}

// template int inc<>(int);

TEST(test_functional , test_applicative){

    // int a = inc(1);
    auto get_int = lift(inc<int>) * natural<int> ;

    auto res = get_int("114514");
    EXPECT_TRUE(res);
    EXPECT_EQ(res->first , 114515);
    EXPECT_EQ(res->second , ""sv);
}

TEST(test_combinators , test_char){
    auto res = upper("Hello");
    constexpr auto expect_res = std::pair{'H' , "ello"sv};
    EXPECT_TRUE(res);
    EXPECT_EQ(res, expect_res);

    auto res2 = lower("Hello");
    EXPECT_FALSE(res2);

    auto res3 = digit("123aaa");
    EXPECT_TRUE(res3);
    constexpr auto exp_res3 = std::pair{'1' , "23aaa"sv};
    EXPECT_EQ(res3.value() , exp_res3);

    auto res4 = letter("a3333");
    EXPECT_TRUE(res4);
    constexpr auto exp_res4 = std::pair{'a' ,"3333"sv};
    EXPECT_EQ(res4.value() , exp_res4);

    auto res5 = alphanum("3aaa");
    EXPECT_TRUE(res5);
    constexpr auto exp_res5 = std::pair{'3' , "aaa"sv};
    EXPECT_EQ(res5.value() , exp_res5);
}

TEST(test_combinators , test_string){
    
    auto res = word("hh!"sv);
    EXPECT_TRUE(res);
    EXPECT_EQ(res.value().first , "hh"sv);

    auto res2 = string("")("hello");
    constexpr auto expect_result2 = std::pair{""sv , "hello"sv};
    EXPECT_EQ(res2, expect_result2);
    
    auto res3 = string("hh")("hheelo");
    constexpr auto expect_result3 = std::pair{"hh"sv , "eelo"sv}; 
    EXPECT_EQ(res3, expect_result3 );

    auto res4 = word("hh"sv);
    EXPECT_TRUE(res4);
    EXPECT_EQ(res4->first , "hh"sv);
    EXPECT_EQ(res4->second , ""sv);

    auto res5 = string("hello")("hello");
    EXPECT_TRUE(res5);
    EXPECT_EQ(res5->first , "hello");
    EXPECT_EQ(res5->second , "");
}

TEST(test_combinators , test_many){

    auto res = many(digit)("112hhh");
    auto result_list = std::forward_list{'1' , '1' , '2'};

    ASSERT_TRUE(res);

    auto && [data , res_str] = res.value(); 
    EXPECT_EQ(data , result_list);
}

TEST(test_combinators , test_nums){
    auto res = natural<int>("1234 aa");
    EXPECT_TRUE(res);
    EXPECT_EQ(res.value().first , 1234);

    res = negative<int>("-1234 aa");
    EXPECT_TRUE(res);
    EXPECT_EQ(res.value().first , -1234);

    res = integer<int>("-1234 aa");
    EXPECT_TRUE(res);
    EXPECT_EQ(res.value().first , -1234);

    auto ints_res = ints<int>("[1,1]");
    EXPECT_TRUE(ints_res);
    auto ints_ls = std::forward_list{1,1};
    EXPECT_EQ(ints_res.value().first , ints_ls);
}

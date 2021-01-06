#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <numeric>
#include <gtest/gtest.h>

#include "parsec.h"

using namespace std::literals;
using namespace pscpp;
using cexpr::vector;

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

    auto res3_ = '1'_char (" 111");
    EXPECT_FALSE(res3_);

    auto res4 = "233"_str ("2333333");
    EXPECT_TRUE(res4);
    EXPECT_EQ(res4->first , "233");
    EXPECT_EQ(res4->second , "3333");

    auto res5 = alphanum("544");
    EXPECT_TRUE(res5);
    EXPECT_EQ(res5->first , '5');

    auto res6 = spaces("   111");
    EXPECT_TRUE(res6);
    EXPECT_EQ(res6->first , none_t{});
    EXPECT_EQ(res6->second , "111"sv);

    auto res7 = many1(digit)("12345");
    EXPECT_TRUE(res7);

    auto && [v , rest ] = *res7;
    auto expect = vector({'1','2','3','4','5'});
    EXPECT_EQ(v , expect);

    auto s = chars(digit)("114514");
    EXPECT_EQ(s->first , "114514"sv);
    EXPECT_EQ(s->second , ""sv);

    auto eofres = (spaces << eof) ("  1111");
    EXPECT_FALSE(eofres);
}

TEST(test_v1 , test_combinator){
    //result , fmap , bind , lift , plus
    // >> << >>= |

    auto res = ('['_char >> digit << ']'_char) ("[1]");
    EXPECT_TRUE(res);
    EXPECT_EQ(res->first , '1');

    auto add_dg = fmap(std::plus<char>{} , digit , digit);
    auto res2 = add_dg("123");
    EXPECT_TRUE(res2);
    EXPECT_EQ(res2->first , '1'+'2');
    EXPECT_EQ(res2->second , "3"sv);

    auto fold_res = foldl_parse("   111" , ' '_char , 0 , [](int i , auto _){return i + 1;});
    EXPECT_TRUE(fold_res);
    EXPECT_EQ(fold_res->first , 3);
    EXPECT_EQ(fold_res.value().second , "111"sv);

    auto option_res = (option('+'_char) >> digit)("123");
    EXPECT_TRUE(option_res);
    EXPECT_EQ(option_res->first , '1');
}

TEST(test_v1 , test_constexpr){
    static_assert( ("111"_str ("11111")));
}

namespace {

constexpr int to_int(std::string_view nums){
    auto v = nums | std::ranges::views::transform([](char ch)->int {return ch - '0';}) ;
    return std::accumulate(v.begin() , v.end(), 0 , [](int init , int i ){return init * 10 + i;});
}

constexpr int eval_lispy(char op , vector<int> ints){
    switch (op){
    case '+': return std::accumulate(ints.begin() , ints.end() , 0 , std::plus<int>{});
    case '-': return std::accumulate(ints.begin() , ints.end() , 0 , std::minus<int>{});
    case '*': return std::accumulate(ints.begin() , ints.end() , 1 , std::multiplies<int>{});
    case '/':
        if(ints.size()!=2) throw std::invalid_argument{R"(incorrect argument for '/' operation .)"};
        return ints[0] / ints[1];
    default : throw std::domain_error{"unexcept operator."};
    }
}

auto natural = fmap(to_int , chars(digit));
auto negative = fmap(std::negate<int>{} , ('-'_char >> natural)) ;

auto sub_expr(parser_string str)->parser_result<int>; 

auto 
numbers     = negative | natural;
auto 
opt         = spaces >> one_of("+-*/") << spaces;
auto 
expr        = spaces >> (numbers | sub_expr) << spaces;
auto 
lispy       = fmap(eval_lispy , opt , many(expr));
auto 
_sub_expr   = '('_char >> lispy << ')'_char;

auto sub_expr(parser_string str)->parser_result<int>{
    return _sub_expr(str);
}

};

TEST(test_v1 , test_parse_expr){

    auto nat = natural("112");
    EXPECT_EQ(nat.value().first , 112);

    auto neg = negative("-112");
    EXPECT_EQ(neg.value().first , -112);

    auto neg2= negative("11");
    EXPECT_FALSE(neg2);

    auto n = numbers("114514");
    EXPECT_TRUE(n);
    EXPECT_EQ(n.value().first , 114514);

    auto op = opt(" + ");
    EXPECT_TRUE(op);
    EXPECT_EQ(op->first ,'+');

    auto res = expr("114514");
    EXPECT_TRUE(res);
    EXPECT_EQ(res->first , 114514);
    
    auto res1= expr("( + 1 2 3 )");
    EXPECT_TRUE(res1);
    EXPECT_EQ(res1->first , 6);
}
#include <cmath>
#include <gtest/gtest.h>
#include "parser/parsec_experimental.h"

using namespace std::literals;

int mpow(int a , int  b){
    return std::pow(a,b);
}

parser_result<int> expr(parser_string str);

using bin_op_t = std::function<int(int,int)>;

parser_t<bin_op_t> plus_op = onechar('+') 
    >>= [](char ch){return result(std::plus<int>{});};

parser_t<bin_op_t> minus_op = onechar('-') 
    >>= [](char ch){return result(std::minus<int>{});};

parser_t<bin_op_t> add_op = plus_op | minus_op;

parser_t<bin_op_t> exp_op = onechar('^')
    >>= [](char ch){return result<bin_op_t>(mpow);};

parser_t<int> factor = natural<int> | bracket(onechar('(') , expr , onechar(')'));

parser_t<int> term = chainr1(factor , exp_op);

auto _expr = chainl1(term , add_op);

parser_result<int> expr(parser_string str){
    return _expr(str);
}

TEST(test_parser , test_expr_parser){
    auto res_fac = factor("1-");
    auto expect_res = std::pair{1,"-"sv};
    EXPECT_TRUE(res_fac);
    EXPECT_EQ(*res_fac , expect_res);

    auto res_expr = expr("1-2+3-4");
    EXPECT_TRUE(res_expr);
    EXPECT_EQ(res_expr->first , -2);
    EXPECT_EQ(res_expr->second, ""sv);

    auto res_expr2 = expr("1^3-2^2+3-4");
    EXPECT_TRUE(res_expr2);
    EXPECT_EQ(res_expr2->first , -4);
    EXPECT_EQ(res_expr2->second, ""sv);
}
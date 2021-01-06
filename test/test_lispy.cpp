#include <gtest/gtest.h>

#include "lispy.h"

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

constexpr auto default_visitor = [](auto && _){};

TEST(test_lispy , test_types){
    Symbol s = Symbol::Div;
    Number n {1};
    SExpr v{s,n};

    Expr e{v};

    EXPECT_EQ(v.size() , 2);
    v[0] .match( overloaded{
        [&](Symbol x){EXPECT_EQ(x , s);},
        default_visitor,
    });
    v[1] .match( overloaded{
        [&](Number x){EXPECT_EQ(x,n);},
        default_visitor,
    });
}

#include "parsec.h"
using namespace pscpp;
auto expr_(std::string_view str) -> parser_result<Expr> ;

TEST(test_lispy , test_sexpr1){
    auto n = expr_("-123");
    auto s = expr_("/  ");
    auto e = expr_("(+ 1 2)");

    EXPECT_TRUE(n); EXPECT_TRUE(s) ; EXPECT_TRUE(e);

    EXPECT_EQ(n->first , Expr{-123});
    EXPECT_TRUE(n->second.empty());
    EXPECT_EQ(s->first , Expr{Symbol::Div});
    EXPECT_TRUE(s->second.empty());
    EXPECT_TRUE(e->second.empty());
}

TEST(test_lispy , test_sexpr2){
    auto n = parse_expr("-123");
    auto s = parse_expr("/  ");
    auto e = parse_expr("(+ 1 2)");

    ASSERT_TRUE(n && std::holds_alternative<Number>(n->var()));
    ASSERT_TRUE(s && std::holds_alternative<Symbol>(s->var()));
    ASSERT_TRUE(e && std::holds_alternative<SExpr> (e->var()));

    EXPECT_EQ( *n  , Expr{-123});
    EXPECT_EQ( *s  , Expr{Symbol::Div});

    const auto & sexpr = std::get<SExpr>(e->var());
    EXPECT_EQ(sexpr.size() , 3);
    EXPECT_FALSE(sexpr[0].valueless_by_exception());
    EXPECT_EQ(sexpr[0].var() , Expr{Symbol::Add});
}
#include <gtest/gtest.h>

#include "lispy.h"


constexpr auto default_visitor = [](auto && _){};

using cexpr::vector;

TEST(test_lispy , test_types){
    Symbol s = Symbol::Div;
    Number n {1};
    const SExpr v{s,n};

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

    vector<Expr> ve{Symbol::Div};
    EXPECT_EQ(ve.size() , 1);
    EXPECT_EQ(ve.capacity() , 1);
    ve.push_back(Symbol::Div); 

    EXPECT_EQ(ve.capacity() , 2);
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

    //sexpr should be {Add , 1 , 2}
    const auto & sexpr = std::get<SExpr>(e->var());
    EXPECT_EQ(sexpr.size() , 3);
    EXPECT_FALSE(sexpr[0].valueless_by_exception());
    EXPECT_EQ(sexpr[0].var() , Expr{Symbol::Add});
    EXPECT_TRUE(std::holds_alternative<Symbol>(sexpr[0]));
    EXPECT_TRUE(std::holds_alternative<Number>(sexpr[1]));
    EXPECT_TRUE(std::holds_alternative<Number>(sexpr[2]));

    auto e2 = parse_expr("(+1 2)");
    EXPECT_FALSE(e2);
}
 
TEST(test_lisp , test_sexpr3){
    auto es = parse_lispy("+ 1 2 ");
    EXPECT_TRUE(es);
    EXPECT_EQ(es->size() , 3);
}
#include <gtest/gtest.h>

#include "ast.h"
#include "lispy.h"
#include "runtime.h"

using namespace lispy;

using cexpr::vector , cexpr::cow ,cexpr::box ;
using ast::parse;

TEST(test_lispy , test_types){

    ast::Atom a = "+" , b = "-";
    const ast::List ls({a , b});
    ast::SExpr e{ls};
    auto e2 = e;

    EXPECT_EQ(ls->size() , 2);
    EXPECT_TRUE((*ls)[0].holds<ast::Atom>());
    EXPECT_EQ(a , (*ls)[0].get<ast::Atom>());

}

TEST(test_lispy , test_parse){

    EXPECT_TRUE (parse("123").value().holds<ast::Atom>());
    auto res1 = parse("eval");
    ASSERT_TRUE (res1);
    EXPECT_EQ   (res1->get<ast::Atom>() , "eval");
    EXPECT_TRUE (parse("'()").value().holds<ast::Quote>());
    auto empty= parse("()");
    ASSERT_TRUE (empty);
    ASSERT_TRUE (empty.value().holds<ast::List>());
    EXPECT_EQ   (empty.value().get<ast::List>()->size() , 0);
    
    auto res2 = parse("( eval )");
    ASSERT_TRUE (res2);
    EXPECT_TRUE (res2->holds<ast::List>() && res2->get<ast::List>().ref()[0].holds<ast::Atom>());
    EXPECT_EQ   (res2->get<ast::List>().ref()[0].get<ast::Atom>() , "eval");

    EXPECT_TRUE (parse("(eval ( ))"));
    EXPECT_TRUE (parse("(())"));
}

// TEST(test_lispy , test_eval){
    
//     EXPECT_EQ(eval("defvar {x} 100") , "()");
//     EXPECT_EQ(eval("defvar {y} 200") , "()");

//     auto e_x = parse_lispy("x").value();
//     ASSERT_TRUE(std::holds_alternative<SExpr>(e_x));
//     eval_expr(Runtime::environment() , e_x); 
//     EXPECT_TRUE(std::holds_alternative<Number>(e_x));

//     EXPECT_EQ(eval("x") , "100");
//     EXPECT_EQ(eval("y") , "200");

//     EXPECT_EQ(eval("+ x y") , "300");

//     EXPECT_EQ(eval("list 1 2 3 4") , "quote(1 2 3 4)");

//     EXPECT_EQ(eval("defvar {ls} {1 2  3 4}") , "()");
//     EXPECT_EQ(eval("ls") , "quote(1 2 3 4)");
//     EXPECT_EQ(eval("head ls") , "1");
//     EXPECT_EQ(eval("tail ls") , "4");
//     EXPECT_EQ(eval("join {1 2 3} {4 5}") , "quote(1 2 3 4 5)");
//     EXPECT_EQ(eval("eval {list 1 2 3}") , "quote(1 2 3)");
// }
#include <gtest/gtest.h>

#include "ast.h"
#include "lispy.h"
#include "runtime.h"

using namespace lispy;

using cexpr::vector , cexpr::cow ,cexpr::box ;
using ast::parse;

TEST(test_lispy , test_types){

    ast::Symbol a = "+" , b = "-";
    const ast::List ls({a , b});
    ast::SExpr e{ls};
    auto e2 = e;

    EXPECT_EQ(ls->size() , 2);
    EXPECT_TRUE((*ls)[0].holds<ast::Symbol>());
    EXPECT_EQ(a , (*ls)[0].get<ast::Symbol>());

}

TEST(test_lispy , test_parse){

    EXPECT_TRUE (parse("#t").value().holds<ast::Boolean>());
    EXPECT_TRUE (parse("#f").value().holds<ast::Boolean>());
    EXPECT_TRUE (parse("-123").value().holds<ast::Integer>());
    auto res1 = parse("eval");
    ASSERT_TRUE (res1);
    EXPECT_EQ   (res1->get<ast::Symbol>() , "eval");
    EXPECT_TRUE (parse("'()").value().holds<ast::Quote>());
    auto empty= parse("()");
    ASSERT_TRUE (empty);
    ASSERT_TRUE (empty.value().holds<ast::List>());
    EXPECT_EQ   (empty.value().get<ast::List>()->size() , 0);
    
    auto res2 = parse("( eval )");
    ASSERT_TRUE (res2);
    EXPECT_TRUE (res2->holds<ast::List>() && res2->get<ast::List>().ref()[0].holds<ast::Symbol>());
    EXPECT_EQ   (res2->get<ast::List>().ref()[0].get<ast::Symbol>() , "eval");

    EXPECT_TRUE (parse("(eval ( ))"));
    EXPECT_TRUE (parse("(())"));
}

TEST(test_lispy , test_define){
    EXPECT_EQ(Runtime::eval("(define x '1)") , "x");
    EXPECT_EQ(Runtime::eval("x") , "1");

    Runtime::eval("(define x 2)");
    EXPECT_EQ(Runtime::eval("x") , "2");
}

TEST(test_lispy , test_builtin){
    EXPECT_EQ(Runtime::eval("(car '(1 2 3))") , "1");
    EXPECT_ANY_THROW(Runtime::eval("(car '())"));
    EXPECT_ANY_THROW(Runtime::eval("(cdr '())"));

    EXPECT_EQ(Runtime::eval("(cdr '(1 2 3))") , "'(2 3)");
    EXPECT_EQ(Runtime::eval("(cdr '(1))") , "'()");
    EXPECT_EQ(Runtime::eval("(cons '() '(1 2 3))") , "'(() 1 2 3)");

    EXPECT_EQ(Runtime::eval("(eq? '(1 2 3) '(1 2 3))") , "false");
    EXPECT_EQ(Runtime::eval("(eq? 1 1)") , "true");
}

TEST(test_lispy , test_lambda){
    EXPECT_EQ(Runtime::eval("((lambda (x) (car x)) '(1 2 3))" ) ,"1");
}

TEST(test_lispy , test_quote){
    EXPECT_EQ(Runtime::eval("'1") , "1");
    EXPECT_EQ(Runtime::eval("'#t") , "true");
    EXPECT_EQ(Runtime::eval("'lambda") , "'lambda");
    EXPECT_EQ(Runtime::eval("'()") , "'()");
    EXPECT_EQ(Runtime::eval("''1") , "''1");

    EXPECT_EQ(Runtime::eval("(car '((1 2 3) 1))") , "'(1 2 3)" );
    EXPECT_EQ(Runtime::eval("(car '(x y z))") , "'x");
    EXPECT_EQ(Runtime::eval("(car '(car (1 2 3)))") , "'car");
}

// TEST(test_lispy , test_immutable){}
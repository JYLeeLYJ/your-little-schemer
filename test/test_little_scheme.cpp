#include <gtest/gtest.h>

#include "runtime.h"

using namespace lispy;

TEST(test_lispy , test_car_law){
    EXPECT_EQ(Runtime::eval("(car '(a b c))") , "'a");
    EXPECT_EQ(Runtime::eval("(car '((a b c) x y z))") , "'(a b c)");

    EXPECT_ANY_THROW(Runtime::eval("(car 'hotdog)"));
    EXPECT_ANY_THROW(Runtime::eval("(car '())"));

    EXPECT_EQ(Runtime::eval("(car '(((hotdogs)) (and) (pickle) rel ish) )") , "'((hotdogs))");
    EXPECT_NO_THROW(Runtime::eval("(define l '(((hotdogs)) (and) (pickle) rel ish))"));
    EXPECT_EQ(Runtime::eval("(car l)") , "'((hotdogs))");

    Runtime::eval("(define l '(((hotdogs)) (and)))");
    EXPECT_EQ(Runtime::eval("(car (car l))") , "'(hotdogs)");

}

TEST(test_lispy , test_cdr_law){
    Runtime::eval("(define l '(a b c))");
    EXPECT_EQ(Runtime::eval("(cdr l)") , "'(b c)");
    EXPECT_EQ(Runtime::eval("(cdr '((a b c) x y z) )") , "'(x y z)");
    EXPECT_EQ(Runtime::eval("(cdr  '(hamburger) )") , "'()");
    EXPECT_ANY_THROW(Runtime::eval("(cdr  'hotdogs)"));
    EXPECT_ANY_THROW(Runtime::eval("(cdr '()"));

    EXPECT_EQ(Runtime::eval("(car (cdr '((b) (x y) ((c)))))") , "'(x y)");
    EXPECT_EQ(Runtime::eval("(cdr (cdr '((b) (x y) ((c)))))") , "'(((c)))");
    EXPECT_ANY_THROW(Runtime::eval("(cdr (car '(a (b (c)) d)))"));
}

TEST(test_lipsy , test_cons_law){
    EXPECT_EQ(Runtime::eval("(cons 'peanut '(butter and jelly))") , "'(peanut butter and jelly)");
    EXPECT_EQ(Runtime::eval("(cons '(banana and) '(butter and jelly))") , "'((banana and) butter and jelly)");

    Runtime::eval("(define s '(a b (c)))");
    Runtime::eval("(define l '())");

    EXPECT_EQ(Runtime::eval("(cons s l)") , "'((a b (c)))");
    EXPECT_EQ(Runtime::eval("(cons 'a '())") , "'(a)");

    EXPECT_ANY_THROW(Runtime::eval("(cons '((a b c)) 'b)"));
    EXPECT_ANY_THROW(Runtime::eval("(cons 'a 'b)"));

    Runtime::eval("(define l '((b) c d))");

    EXPECT_EQ(Runtime::eval("(cons  'a (car '((b) c d)))") , "'(a b)");
    EXPECT_EQ(Runtime::eval("(cons 'a (cdr l))") , "'(a c d)");
}

TEST(test_lispy , test_asserts){
    EXPECT_EQ(Runtime::eval("(atom? 'atom)") , "true");
    EXPECT_EQ(Runtime::eval("(atom? 'turkey)") , "true");
    EXPECT_EQ(Runtime::eval("(atom? 1492)") , "true");
    EXPECT_EQ(Runtime::eval("(atom? 'u)") , "true");
    EXPECT_EQ(Runtime::eval("(atom? '())") , "false");
    EXPECT_EQ(Runtime::eval("(atom? '(Harry had a heap of apples))") , "false");
    EXPECT_EQ(Runtime::eval("(atom? (car '(Harry had a heap of apples)))") , "true");
    EXPECT_EQ(Runtime::eval("(atom? eval)"), "true");

    // null? in scheme allows non-list type.
    EXPECT_EQ(Runtime::eval("(null? '())") , "true");
    EXPECT_EQ(Runtime::eval("(null? '(a b c))") , "false");
    EXPECT_EQ(Runtime::eval("(null? 'spaghetti )") , "false");

    EXPECT_EQ(Runtime::eval("(eq? (car '(Mary had a little lamb chop)) 'Mary)") , "true");
    EXPECT_EQ(Runtime::eval("(eq? ''1 ''1)") , "false");
    EXPECT_EQ(Runtime::eval("(eq? '#t '#t)") , "true");

    Runtime::eval("(define l '(beans beans we need jelly beans))");
    EXPECT_EQ(Runtime::eval("(eq? (car l) (car (cdr l)))") , "true");
}

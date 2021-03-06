#include <gtest/gtest.h>

#include "runtime.h"

using namespace lispy;
using namespace std::literals;

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

TEST(test_lispy , test_lat){
    Runtime::eval(R"(
        (define lat?
            (lambda (l)
                (cond 
                    ((null? l) #t)
                    ((atom? (car l)) (lat? (cdr l)))
                    (else #f)
        )))
    )");

    std::vector cases = {
        std::pair
        {"(lat? '(Jack Sprat could eat no chicken fat))" , "true"},
        {"(lat? '((Jack) Sprat could eat no chicken fat))","false"},
        {"(lat? '(Jack (Sprat could) eat no chicken fat))" , "false"},
        {"(lat? '())","true"},
        {"(lat? '(bacon (and eggs)))" , "false"},
    };

    for(auto && [input , output] : cases){
        EXPECT_EQ(Runtime::eval(input),output);
    }
}

TEST(test_lispy , test_member){
    Runtime::eval(R"(
        (define member?
            (lambda (a lat) 
            (cond 
                ((null? lat) #f)
                (else 
                    (or 
                        (eq? (car lat) a) 
                        (member? a (cdr lat))
                    )
                )
        )))
    )");

    std::vector cases{
        std::pair
        {"(member? 'tea '(coffee tea or milk))" , "true"},
        {"(member? 'poached '(fried eggs and scrambled eggs))" , "false"},
        {"(member? 'a '())" , "false"},
    };

    for(auto & [ in , out] : cases){
        EXPECT_EQ(Runtime::eval(in) , out);
    }
    EXPECT_ANY_THROW(Runtime::eval("(member? 'a 'b)"));
}

TEST(test_lispy , test_rember){
    //erase
    Runtime::eval(R"(
        (define rember
            (lambda (a lat)
            (cond
                ((null? lat) '())
                (else (cond
                    ( ( eq? ( car lat) a) ( cdr lat))
                    (else (cons (car lat) (rember a ( cdr lat)))))
                )
        )))
    )");

    std::vector cases{
        std::pair
        {"(rember 'mint '(lamb chops and mint jelly))"  , "'(lamb chops and jelly)"},
        {"(rember 'mint '(lamb chops and mint flavored mint jelly))","'(lamb chops and flavored mint jelly)"},
        {"(rember 'toast '(bacon lettuce and tomato))" , "'(bacon lettuce and tomato)"},

    };

    for(auto & [ in , out ]: cases){
        EXPECT_EQ(Runtime::eval(in) , out);
    }
}

TEST(test_lispy , test_first){
    Runtime::eval(R"(
        (define firsts
            (lambda (l)
            (cond
            ((null? l) '())
            (else ( cons ( car ( car l))
            (firsts ( cdr l)))))))
    )");

    std::vector cases{
        std::pair
        {"(firsts '((apple peach pumpkin) (plum pear cherry) (grape raisin pea) (bean carrot eggplant)))", "'(apple plum grape bean)"},
        {"(firsts '((a b) (c d) (e f)))" , "'(a c e)"},
        {"(firsts '())","'()"},
        {"(firsts '(((five plums) four) (eleven green oranges) ((no) more)))" , "'((five plums) eleven (no))"},
    };

    for(auto & [in , out] : cases ){
        EXPECT_EQ(Runtime::eval(in) , out);
    }
}

TEST(test_lispy , test_peano_numbers){
    Runtime::eval(R"(
        (define +
            (lambda ( n m)
            (cond
            ((zero? m) n)
            (else ( add1 (+ n (sub1 m))))))) 
    )");

    Runtime::eval(R"(
        (define -
            (lambda ( n m)
            (cond
            ((zero? m) n)
            (else ( sub1 (- n (sub1 m))))))) 
    )");

    Runtime::eval(R"(
        (define >
            (lambda ( n m)
            (cond
            ((zero? n) #f)
            ((zero? m) #t)
            (else (> (sub1 n) (sub1 m)))))) 
    )");

    Runtime::eval(R"(
        (define <
            (lambda ( n m)
            (cond
            ((zero? m) #f)
            ((zero? n) #t )
            (else ( < (sub1 n) (sub1 m)))))) 
    )");

    Runtime::eval(R"(
        (define =
            (lambda ( n m)
            (cond
            ((> n m) #f)
            ((< n m) #f )
            (else #t ))) ) 
    )");

    std::vector case1{
        std::pair
        {"(+ 46 12)" , "58"},
        {"(- 14 3)" , "11"},
        {"(- 17 9)" , "8"},
        {"(- 18 25)" ,"-7"},    //practicallly allow non natural number
        {"(> 1 3)" , "false"},
        {"(< 1 1)" , "false"},
        {"(= 2 2)" , "true"},
        {"(= 1 10)" , "false"}
    };

    for(auto & [in , out] : case1){
        EXPECT_EQ(Runtime::eval(in), out);
    }
}

TEST(test_lispy , test_y){
    auto define_Y = R"(
    (define Y
        (lambda (F)
        ((lambda (f)
            (lambda (n)
            ((F (f f)) n)))
        (lambda (f)
            (lambda (n)
            ((F (f f)) n)))))

    )
    )"sv;

    auto define_fibb = R"(
    (define fibb  
        (lambda (fib)
        (lambda (n)
            (cond 
                ((= n 0) 0)
                ((= n 1) 1)
                (else (+ (fib (- n 1)) (fib (- n 2))))
            )
        ))
    )
    )"sv;

    Runtime::eval(define_Y);
    Runtime::eval(define_fibb);

    EXPECT_EQ(Runtime::eval("((Y fibb) 0)"),"0");
    EXPECT_EQ(Runtime::eval("((Y fibb) 1)"),"1");
    EXPECT_EQ(Runtime::eval("((Y fibb) 5)"),"5");
}
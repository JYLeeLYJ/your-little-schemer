
#include "lispy.h"

Expr eval_sexpr(SExpr & sexpr){
    auto & [s] = sexpr;
    if(s.size() == 0 || std::holds_alternative<Symbol>(s[0]) == false) 
        return std::move(sexpr);

    if(s.size() == 1) 
        return std::move(s[0]);

    for(auto & e : s) 
        eval(e);

    // all parameters should be number type
    auto cond = std::all_of(s.begin() + 1, s.end() , [](const Expr & e ){ return std::holds_alternative<Number>(e);} );
    if(cond == false ) 
        throw std::invalid_argument{"all parameter for symbol should be number."};
    if(s.size() > 3) 
        throw std::invalid_argument{"too much parameter for operator +-*/. "};

    auto lhs  = std::get<Number>(s[1]) ,  rhs = std::get<Number>(s[2]);
    switch (std::get<Symbol>(s[0])){  
    case Symbol::Add : return Number{ lhs + rhs };
    case Symbol::Sub : return Number{ lhs - rhs };
    case Symbol::Mut : return Number{ lhs * rhs };
    case Symbol::Div : 
        if(rhs == 0) throw std::domain_error{"divied 0 error."};
        return Number{ lhs / rhs };
    };
    return 0; // unreach
}

void eval(Expr & e){
    e.match(overloaded{
        [&] (SExpr & s) { e = eval_sexpr(s);},
        []  (auto && _) { /*donothing */ },
    });
}

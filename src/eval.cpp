#include <string>
#include <ranges>
#include <iterator>
#include <functional>
#include <fmt/format.h>

#include "lispy.h"
#include "runtime.h"

struct Runtime : protected lispy::Environment{
private:
    Runtime() = default;
public:
    static lispy::Environment & environment() {
        static Runtime env{};
        return env;
    }
};

namespace lispy {

Expr eval_sepxr(SExpr & sexpr) {
    auto && [s] = sexpr;
    for(auto & e : s) eval_expr(e);

    if(s.size() == 0) return std::move(sexpr);

    auto & env = Runtime::environment();
    auto & head = s[0];
    if(std::holds_alternative<Function>(head)){
        auto & fn = *env.get_function(std::get_if<Function>(&head)->name);
        return fn(env , ExprListView{s.begin() + 1 ,s.end()});
    }
    else if(s.size() == 1) 
        return std::move(s[0]);
    else
        throw std::domain_error{"first element is not a function"};
}

void eval_expr(Expr & e){
    e.match(overloaded{
        [&](Symbol s)       {
            if(!Runtime::environment().contains_symbol(s.identifier)) 
                throw std::invalid_argument{fmt::format("unknown symbol {}" , s.identifier)};

            if(auto * pvar = Runtime::environment().get_var(s.identifier)
            ; pvar)
                e = *pvar;
            else 
                e = Function{s.identifier};
        },
        [&](SExpr & sexpr)  { e = eval_sepxr(sexpr);},
        [] (auto && _) { /* do nothing*/}
    });
}

std::string eval(std::string_view input){
    auto result = parse_lispy(input);
    if(!result) 
        return "ParseError : invalid input format.";

    eval_expr(*result);
    return print_expr(*result);
}

}
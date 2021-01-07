#include <variant>
#include <string_view>
#include <optional>
#include <numeric>
#include <stdexcept>
#include <fmt/format.h>

#include "parsec.h"
#include "lispy.h"

using namespace pscpp;

namespace {

Expr to_number(std::optional<char> op, std::string_view nums){
    auto v = nums | std::ranges::views::transform([](char ch)->int {return ch - '0';}) ;
    int n = std::accumulate(v.begin() , v.end(), 0 , [](int init , int i ){return init * 10 + i;});
    return Number{op ? -n : n};
}

Expr to_sym(char c){ 
    return static_cast<Symbol>(c);
}

Expr to_sexpr(cexpr::vector<Expr> && e){
    return std::move(e);
}

/*                                           \
    number : /-?[0-9]+/ ;                    \
    symbol : '+' | '-' | '*' | '/' ;         \
    sexpr  : '(' <expr>* ')' ;               \
    expr   : <number> | <symbol> | <sexpr> ; \
    lispy  : /^/ <expr>* /$/ ;               \
*/

auto _expr(std::string_view str) -> parser_result<Expr> ;


auto numbers    = fmap(to_number    , option('-'_char)  , chars(digit));
auto symbol     = fmap(to_sym       , one_of("+-*/"));
auto expr_ls    = spaces >> sepby(_expr , spaces1) << spaces ;
auto sexpr      = fmap(to_sexpr     , '('_char >> expr_ls << ')'_char) ;
auto expr       = numbers | symbol | sexpr ;// ;
auto lispy      = expr_ls << eof; 

auto _expr(std::string_view str) -> parser_result<Expr> {
    return expr(str);
}
 
}


int eval_expr(const Expr & e){
    return e.match<int>(overloaded{
        [](const SExpr & e) { return  eval_lispy(e); },
        [](Number n)        { return n;},
        [](auto && v)       { 
            throw std::invalid_argument{"this argument cannot be only operator symbol."};
            return 0;
        },
    });
}

int eval_lispy(const SExpr & e){
    if(e.empty()) 
        throw std::invalid_argument{"this S-expression cannot be empty."};
    
    if(e.size() == 1)
        return eval_expr(e[0]);
    if(!std::holds_alternative<Symbol>(e[0])) 
        throw std::invalid_argument{"First argument of this argument should be operator symbol."};

    switch (std::get<Symbol>(e[0])){
    case Symbol::Add :
        return std::accumulate(e.begin() + 1 ,e.end() , 0 , [](int i, const Expr & e) { return i + eval_expr(e);});
    case Symbol::Mut : 
        return std::accumulate(e.begin() + 1 ,e.end() , 1 , [](int i, const Expr & e){return i * eval_expr(e);});
    case Symbol::Minus:
        if(e.size() == 2)return - eval_expr(e[1]);
        else if(e.size() != 3) throw std::invalid_argument{R"( operator '-' needs 2 operands)"};
        return eval_expr(e[1]) - eval_expr(e[2]);
    case Symbol::Div:{
        if(e.size() != 3) throw std::invalid_argument{R"( operator '/' needs 2 operands)"};
        int m = eval_expr(e[3]);
        if(m == 0 ) throw std::invalid_argument{"divide by 0 error."};
        return eval_expr(e[2]) / m ;
    }
    default:
        throw std::domain_error{"invalid symbol."};
    }
}

std::optional<SExpr> parse_lispy(std::string_view str){
    auto res = lispy(str);
    if(res) return std::move(res->first);
    else return {};
}

std::optional<Expr> parse_expr(std::string_view str) {
    auto res = expr(str);
    if(res) return std::move(res->first);
    else return {};
} 

std::string print_tree(const Expr & e){
    return "not yet complete.";
}
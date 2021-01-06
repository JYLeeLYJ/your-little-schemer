#include <variant>
#include <string_view>
#include <optional>
#include <numeric>
#include <fmt/format.h>

#include "parsec.h"
#include "lispy.h"

/*                                           \
    number : /-?[0-9]+/ ;                    \
    symbol : '+' | '-' | '*' | '/' ;         \
    sexpr  : '(' <expr>* ')' ;               \
    expr   : <number> | <symbol> | <sexpr> ; \
    lispy  : /^/ <expr>* /$/ ;               \
*/
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
    return SExpr{std::move(e)};
}

}

auto expr_(std::string_view str) -> parser_result<Expr> ;

namespace {

auto numbers    = fmap(to_number    , option('-'_char)  , chars(digit));
auto symbol     = fmap(to_sym       , one_of("+-*/"));
auto sexpr      = fmap(to_sexpr     , '('_char >> many1(expr_) << ')'_char) ;
auto expr       = spaces >> (numbers | symbol | sexpr ) << spaces ;
auto lispy      = many1(expr) << eof;

}

auto expr_(std::string_view str) -> parser_result<Expr> {
    return expr(str);
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
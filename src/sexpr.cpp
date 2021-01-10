#include <variant>
#include <optional>
#include <numeric>
#include <stdexcept>

#include "parsec.h"
#include "lispy.h"

using namespace pscpp;

namespace {

Expr to_number(std::optional<char> op, std::string_view nums){
    int n = std::accumulate(nums.begin() , nums.end(), 0 , [](int init , char i ){return init * 10 + ( i - '0');});
    return Number{op ? -n : n};
}

Expr to_sym(char c){ 
    return static_cast<Symbol>(c);
}

Expr to_sexpr(ExprList && e){
    return SExpr{std::move(e)};
}

Expr to_quote(ExprList && q){
    return Quote{std::move(q)};
}

/*                                                     \
    number : /-?[0-9]+/ ;                              \
    symbol : '+' | '-' | '*' | '/' ;                   \
    sexpr  : '(' <expr>* ')' ;                         \
    qexpr  : '{' <expr>* '}' ;                         \
    expr   : <number> | <symbol> | <sexpr> | <qexpr> ; \
    lispy  : /^/ <expr>* /$/ ;                         \
*/

auto _expr(std::string_view str) -> parser_result<Expr> ;

auto numbers    = fmap(to_number    , option('-'_char)  , chars(digit));
auto symbol     = fmap(to_sym       , one_of("+-*/"));
auto expr_ls    = spaces >> sepby(_expr , spaces1) << spaces ;
auto sexpr      = fmap(to_sexpr     , '('_char >> expr_ls << ')'_char) ;
auto quote      = fmap(to_quote     , '{'_char >> expr_ls << '}'_char) ;
auto expr       = numbers | symbol | sexpr | quote ;// ;
auto lispy      = expr_ls << eof ; 

auto _expr(std::string_view str) -> parser_result<Expr> {
    return expr(str);
}
 
}

std::optional<Expr> parse_lispy(std::string_view str){
    auto result = lispy(str);
    if(!result)  return {};
    
    auto & expr_list = result.value().first;
    if(expr_list.size() == 1) return std::move(expr_list[0]);
    else return SExpr{std::move(expr_list)};
}

std::optional<Expr> parse_expr(std::string_view str){
    auto result = expr(str);
    if(result) return result.value().first;
    else return {};
}
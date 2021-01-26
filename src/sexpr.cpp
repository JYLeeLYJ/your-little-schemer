#include <variant>
#include <optional>
#include <numeric>

#include "parsec.h"
#include "ast.h"
#include "lispy.h"

using namespace pscpp;
using namespace lispy;

namespace {

ast::SExpr from_symbol(std::string_view s){
    return s;
}
ast::SExpr from_list(cexpr::vector<ast::SExpr> &&list){
    return ast::List{std::move(list)};
}
ast::SExpr from_quote(ast::SExpr s){
    return ast::Quote{s};
}
ast::SExpr from_boolean(char b){
    return ast::Boolean{b == 't'};
}
ast::SExpr from_integer(std::optional<char> neg , std::string_view nums){
    auto ints = std::accumulate(
        nums.begin() , nums.end(), ast::Integer{0} , 
        [](ast::Integer init , char i ){return init * 10 + ( i - '0');});
    return neg? -ints : ints;
}

/*                                                    
    integer: /-?[0-9]+/ ;                              
    boolean: #t | #f ;
    symbol : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!?&]+/ ;        
    sexpr  : <symbol> | <quote> | <list>   ;             
    quote  : '\'' <sexpr>       ;                      
    list   : '(' <sexpr>* ')'   ;                      
    lispy  : /^/ <sexpr> /$/ ;                           
*/ 
auto sexpr(std::string_view str) -> parser_result<ast::SExpr> ;

auto integer    = fmap(from_integer , option('-'_char) , chars(digit));
auto boolean    = fmap(from_boolean , '#'_char >> ( one_of("tf")));
auto literal    = boolean | integer ;

auto symbol     = fmap(from_symbol  , chars(letter | one_of("_+-*/\\=<>!?&")));
auto list       = fmap(from_list    ,'('_char >> spaces >> sepby(sexpr , spaces1) << spaces << ')'_char);
auto quote      = fmap(from_quote   ,'\''_char >> sexpr);
auto sexpr_impl = ( literal | symbol | quote | list );
auto lispy_     = spaces >> sexpr_impl << eof;

auto sexpr(std::string_view str) -> parser_result<ast::SExpr>{
    return sexpr_impl(str);
}

}

namespace lispy{

std::optional<ast::SExpr> ast::parse(std::string_view input) {
    auto result = lispy_(input);
    return result? std::optional{result->first} : std::nullopt ;
}

}


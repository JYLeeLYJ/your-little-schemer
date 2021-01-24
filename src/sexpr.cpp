#include <variant>
#include <optional>

#include "parsec.h"
#include "ast.h"
#include "lispy.h"

using namespace pscpp;
using namespace lispy;

namespace {

ast::SExpr from_atom(std::string_view s){
    return s;
}
ast::SExpr from_list(cexpr::vector<ast::SExpr> &&list){
    return ast::List{std::move(list)};
}
ast::SExpr from_quote(ast::SExpr s){
    return ast::Quote{s};
}

/*                                                    
    integer: /-?[0-9]+/ ;                              

    atom   : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;        
    sexpr  : <atom> | <quote> | <list>   ;             
    quote  : '\'' <sexpr>       ;                      
    list   : '(' <sexpr>* ')'   ;                      
    lispy  : /^/ <sexpr> /$/ ;                           
*/ 
auto sexpr(std::string_view str) -> parser_result<ast::SExpr> ;

auto atom       = fmap(from_atom    , chars(letter | digit | one_of("_+-*/\\=<>!&")));
auto list       = fmap(from_list    ,'('_char >> spaces >> sepby(sexpr , spaces1) << spaces << ')'_char);
auto quote      = fmap(from_quote   ,'\''_char >> sexpr);
auto sexpr_impl = (atom | quote | list );
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


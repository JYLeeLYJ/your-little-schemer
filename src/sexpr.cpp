#include <variant>
#include <string_view>
#include <optional>
#include <numeric>
#include <stdexcept>
#include <iterator>
#include <fmt/format.h>
#include <fmt/ranges.h>

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

Expr to_sexpr(SExpr && e){
    return std::move(e);
}

Expr to_quote(cexpr::vector<Expr> && q){
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
    else return SExpr(std::move(expr_list));
}

std::optional<Expr> parse_expr(std::string_view str){
    auto result = expr(str);
    if(result) return result.value().first;
    else return {};
}

struct default_format_parser{
    constexpr auto parse(fmt::format_parse_context & ctx){
        auto it = ctx.begin() , end = ctx.end();
        if (it != end && *it != '}')  throw fmt::format_error("invalid format");
        return it;
    }
};
template<>
struct fmt::formatter<Quote> : default_format_parser{
    template<class Context >
    auto format(const Quote & q , Context & ctx){
        return format_to(ctx.out() , "quote({})" , fmt::join(q.exprs.begin(), q.exprs.end() , " "));
    }
};
template<>
struct fmt::formatter<SExpr> : default_format_parser{
    template<class Context>
    auto format(const SExpr & s , Context & ctx){
        return format_to(ctx.out() , "({})" , fmt::join(s.begin(), s.end() , " "));
    }
};
template<>
struct fmt::formatter<Symbol> : default_format_parser{
    template<class Context>
    auto format(Symbol s , Context & ctx){
        return format_to(ctx.out() , "{}" , static_cast<char>(s));
    }
};

template<>
struct fmt::formatter<Expr> : default_format_parser{
    template<class Context>
    auto format (const Expr & s , Context & ctx){
        using RetIt = decltype(ctx.out());
        return s.match<RetIt>([&](auto && e) mutable{
            return format_to(ctx.out() , "{}" , e);
        });
    }  
};

// std::string print_tree(const Expr & e){
//     return "not yet complete.";
// }

std::string print_expr(const Expr & e){
    return fmt::format("{}" , e);
}

Expr eval_sexpr(SExpr & s){
    if(s.size() == 0 || std::holds_alternative<Symbol>(s[0]) == false) 
        return std::move(s);

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

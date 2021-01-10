#include <fmt/format.h>
#include <fmt/ranges.h>

#include "lispy.h"

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
    auto format(const SExpr & sexpr , Context & ctx){
        auto & [s] = sexpr;
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

std::string print_expr(const Expr & e){
    return fmt::format("{}" , e);
}

#include <fmt/format.h>
#include <fmt/ranges.h>

#include "lispy.h"
#include "ast.h"

using namespace lispy;
using namespace std::literals;

struct default_format_parser{
    constexpr auto parse(fmt::format_parse_context & ctx){
        auto it = ctx.begin() , end = ctx.end();
        if (it != end && *it != '}')  throw fmt::format_error("invalid format");
        return it;
    }
};
template<>
struct fmt::formatter<ast::Quote> : default_format_parser{
    template<class Context >
    auto format(const ast::Quote & q , Context & ctx){
        return format_to(ctx.out() , "'{}" , q.ref());
    }
};
template<>
struct fmt::formatter<ast::List> : default_format_parser{
    template<class Context>
    auto format(const ast::List & list , Context & ctx){
        return format_to(ctx.out() , "({})" , fmt::join(list->begin(), list->end() , " "));
    }
};

template<>
struct fmt::formatter<ast::Lambda> : default_format_parser{
    template<class Context>
    auto format(const ast::Lambda & f , Context & ctx){
        return format_to(ctx.out() , "#<procedure>");
    }
};

template<>
struct fmt::formatter<ast::BuiltinFn> : default_format_parser{
    template<class Context>
    auto format(const ast::BuiltinFn & f , Context & ctx){
        return format_to(ctx.out() , "#<builtin procedure:{}>" , f.name);
    }
};


template<>
struct fmt::formatter<ast::SExpr> : default_format_parser{
    template<class Context>
    auto format (const ast::SExpr & s , Context & ctx){
        using RetIt = decltype(ctx.out());
        return s.match<RetIt>([&](auto && e) mutable{
            return format_to(ctx.out() , "{}" , e);
        });
    }  
};

namespace lispy {

std::string ast::print_sexpr(const ast::SExpr & e){
    return fmt::format("{}" , e);
}

}

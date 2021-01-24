#pragma once
#include <string_view>
#include <variant>
#include <optional>

#include "constexpr_containers.hpp"
#include "utils.hpp"

namespace lispy{

namespace ast{
    
    struct SExpr;

    using Atom = std::string_view ;
    using Quote= cexpr::cow<SExpr>;
    using List = cexpr::cow<cexpr::vector<SExpr>>;

    struct Lambda {
        cexpr::cow<cexpr::vector<SExpr>> bounded;
        cexpr::cow<cexpr::vector<std::string_view>> var_names;
        cexpr::cow<SExpr>   body;
    };

    struct BuiltinFn{
        std::string_view name;
    };
    using SExprBase = variant_base<Atom,Quote,List,Lambda,BuiltinFn>;
    struct SExpr : SExprBase {
        using SExprBase::variant_base;
        bool operator ==(const SExpr & ) const = delete;
    };

    std::optional<ast::SExpr> parse(std::string_view ) ;

    std::string print_sexpr(const ast::SExpr &) ;

}

}



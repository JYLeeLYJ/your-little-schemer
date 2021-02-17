#pragma once
#include <string_view>
#include <variant>
#include <optional>

#include "constexpr_containers.hpp"
#include "utils.hpp"

namespace lispy{

namespace ast{
    
    struct SExpr;

    using Symbol    = std::string_view ;
    using Quote     = cexpr::cow<SExpr>;
    using List      = cexpr::cow<cexpr::vector<SExpr>>;

    using Boolean   = bool;
    using Integer   = int64_t;
    // using String    = std::string;

    struct Lambda {
        cexpr::cow<cexpr::vector<SExpr>> bounded;
        cexpr::cow<cexpr::vector<std::string_view>> var_names;
        cexpr::cow<cexpr::vector<std::pair<std::string_view,SExpr>>> free_vars;
        cexpr::cow<SExpr>   body;
        bool operator ==(const Lambda &) const = default;
    };

    struct BuiltinFn{
        std::string_view name;
        bool operator ==(const BuiltinFn &) const = default;
    };
    using SExprBase = variant_base<Integer,Boolean,Symbol,Quote,List,Lambda,BuiltinFn>;
    struct SExpr : SExprBase {
        using SExprBase::variant_base;
        bool operator ==(const SExpr & ) const = default;
    };

    std::optional<ast::SExpr> parse(std::string_view ) ;

    std::string print_sexpr(const ast::SExpr &) ;

}

}



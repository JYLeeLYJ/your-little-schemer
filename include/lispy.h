#pragma once

#include <optional>
#include <string_view>
#include <variant>
#include <string>

#include "constexpr_containers.hpp"

std::optional<int> parse_polish(std::string_view str) ;

using Number = int;
enum class Symbol{  Add = '+',Minus = '-', Mut = '*', Div = '/',};

struct Expr;
struct SExpr : cexpr::vector<Expr>{
    using super = cexpr::vector<Expr>;
    using super::vector;
    constexpr SExpr(cexpr::vector<Expr> && v) noexcept 
    : super{std::move(v)}{}
};

struct Expr : std::variant<Number , Symbol , SExpr>{
    using super = std::variant<Number , Symbol , SExpr>;
    using super::variant;

    template<class V>
    constexpr auto match(V && vis){ 
        return std::visit( std::forward<V>(vis) , static_cast<typename Expr::super &>(*this));
    }
    template<class V>
    constexpr auto match(V && vis) const {
        return std::visit( std::forward<V>(vis) , static_cast<const typename Expr::super &>(*this));
    }
    template<class R , class V>
    constexpr R match(V && vis){
        return std::visit<R>(std::forward<V>(vis) , static_cast<typename Expr::super &>(*this));
    }
    template<class R , class V>
    constexpr R match(V && vis) const {
        return std::visit( std::forward<V>(vis) , static_cast<const typename Expr::super &>(*this));
    }

    constexpr super & var(){return *this;}
    constexpr const super & var() const {return * this;}
    constexpr bool operator== (const Expr & rhs){ return var() == rhs.var();}
};



std::optional<SExpr> parse_lispy(std::string_view str);

std::optional<Expr> parse_expr(std::string_view);
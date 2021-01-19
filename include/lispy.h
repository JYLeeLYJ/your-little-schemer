#pragma once

#include <optional>
#include <string_view>
#include <variant>
#include <string>

#include "constexpr_containers.hpp"

namespace lispy{


std::optional<int> parse_polish(std::string_view str) ;

using Number = int;

struct Symbol {
    std::string_view identifier;
    constexpr bool operator == (const Symbol & s) const = default;
};

struct Function{
    std::string_view name;
    constexpr bool operator == (const Function & s) const = default;
};

struct Variable{};

struct Expr;
using ExprList = cexpr::vector<Expr> ;
struct SExpr{
    ExprList exprs;
    constexpr bool operator == (const SExpr & q) const = default;
};

struct Quote{
    ExprList exprs;
    constexpr bool operator == (const Quote & q) const = default;
};

using ExprBase = std::variant<Number , Symbol , SExpr , Quote ,Function>;
struct Expr : ExprBase{
    using ExprBase::variant;

    template<class V>
    constexpr auto match(V && vis) const { 
        return std::visit( std::forward<V>(vis) , var());
    }
    template<class V>
    constexpr auto match(V && vis){
        return std::visit(std::forward<V>(vis), var());
    }
    template<class R , class V>
    constexpr R match(V && vis)  {
        return std::visit<R>(std::forward<V>(vis) , var());
    }
    template<class R , class V>
    constexpr R match(V && vis) const {
        return std::visit<R>(std::forward<V>(vis) , var());
    }

    template<class T>
    constexpr bool holds() const {
        return std::holds_alternative<T>(*this);
    }

    template<class T>
    constexpr decltype(auto) get() {
        return std::get<T>(var());
    }

    constexpr ExprBase & var(){return *this;}
    constexpr const ExprBase & var() const {return *this;}
    constexpr bool operator== (const Expr & rhs){ return var() == rhs.var();}
};

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

std::optional<Expr> parse_lispy(std::string_view);

std::string print_expr(const Expr &);

void eval_expr(Expr & );

std::string eval(std::string_view);

}

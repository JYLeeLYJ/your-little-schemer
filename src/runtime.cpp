#include <fmt/format.h>
#include <typeinfo>
#include <ranges>

#include "runtime.h"

// add builtins 

using namespace lispy;

namespace{

template<class T> 
const char * name () {
    if constexpr (std::is_same_v<T , SExpr>) 
        return "SExpr" ;
    else if constexpr (std::is_same_v<T , Number>) 
        return "Number";
    else if constexpr (std::is_same_v<T , Symbol>)
        return "Symbol";
    else if constexpr (std::is_same_v<T , Quote>)
        return "Quote";
    else if constexpr (std::is_same_v<T , Function>)
        return "Function";
    else 
        typeid(T).name();
}

inline void check_param_nums(int sz , const ExprListView & view){
    if(view.size() != sz) 
        throw std::invalid_argument{fmt::format("incorrect parameter number , expect {}" , sz)};
}

template<std::ranges::range R>
inline void check_param_no_empty(R & range) {
    if(range.empty())
        throw std::invalid_argument{"sexpr cannot be empty."};
}

template<class T>
inline void check_expr_type(const Expr & e) {
    if(!e.holds<T>())
        throw std::domain_error{
            fmt::format("{} should have type {}" , print_expr(e) , name<T>())};
}

template<class T>
inline void check_param_types(const ExprListView & view){
    for(auto & e : view)
        check_expr_type<T>(e);
}

template<class BinOp>
requires 
    std::is_default_constructible_v<BinOp>  && 
    requires (BinOp b){ {b( 1 , 2)} -> std::integral ; }
Expr builtin_bin_op(Environment & _ , ExprListView view) {
    check_param_nums (2 , view);
    check_param_types<Number>(view);

    auto it = view.begin();
    return BinOp{}(std::get<Number>(*it)  , std::get<Number>(*(it+1)));
}

Expr builtin_list(Environment & _ , ExprListView view) {
    check_param_nums(0 , view);
    return Quote{};
}

Expr builtin_head(Environment & _ , ExprListView view) {
    check_param_nums(1 , view);
    check_param_types<Quote>(view);
    auto & [q] = std::get<Quote>(*view.begin());
    check_param_no_empty(q);
    return q[0];
}

Expr builtin_tail(Environment & _  ,ExprListView view) {
    check_param_nums(1 , view);
    check_param_types<Quote>(view);
    auto & [q] = std::get<Quote>(*view.begin());
    check_param_no_empty(q);
    return q[q.size() -1];
}

Expr builtin_eval(Environment & _ , ExprListView view){
    check_param_nums(1 , view);
    check_param_types<Quote>(view);

    Expr sexpr = SExpr{std::get<Quote>(*view.begin()).exprs};
    eval_expr(sexpr);
    return sexpr;
}

Expr builtin_defvar(Environment & env , ExprListView view){
    check_param_nums(2 , view);

    auto & head = *view.begin();
    auto & var = *(view.begin() + 1);

    //head must be a symbol
    check_expr_type<Quote>(head);
    auto & [q] = std::get<Quote>(head);
    check_param_nums(1 , ExprListView{q.begin() , q.end()});
    check_expr_type<Symbol>(*q.begin());
    auto & symbol = std::get<Symbol>(*q.begin()).identifier ;
    
    env.add_variable(symbol , var);
    return SExpr{};
}

Expr builtin_join(Environment & _ , ExprListView view){
    check_param_no_empty(view);
    check_param_types<Quote>(view);

    ExprList ls{};
    for(auto && q : view){
        auto && [exprs] = std::get<Quote>(q);
        for(auto & e : exprs)
            ls.emplace_back(e);
    }
    return Quote{std::move(ls)};
}

}

//builtin initialize
Environment::Environment() {
    add_function("+" , builtin_bin_op<std::plus<Number>>);
    add_function("-" , builtin_bin_op<std::minus<Number>>);
    add_function("*" , builtin_bin_op<std::multiplies<Number>>);
    add_function("/" , builtin_bin_op<std::divides<Number>>);

    add_function("list" , builtin_list);
    add_function("head" , builtin_head);
    add_function("tail" , builtin_tail);
    add_function("eval" , builtin_eval);
    add_function("join" , builtin_join);
    add_function("defvar",builtin_defvar);
}
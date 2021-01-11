#include <unordered_map>
#include <string>
#include <ranges>
#include <iterator>
#include <functional>

#include <fmt/format.h>
#include "lispy.h"

struct ExprListView {
    ExprList::const_iterator _beg , _end;
    
    auto begin()const   {return _beg;}
    auto end()  const   {return _end;}

    bool empty() const  {return _end == _beg;}
    auto size() const   {return _end - _beg;}
};

struct Enviroment;
using LispyFunction = std::function<Expr (Enviroment &, ExprListView)>;
using Closure = std::unordered_map<std::string , LispyFunction>;

struct Enviroment{
    Closure closure;

    explicit Enviroment () ;

    bool contains_symbol(std::string_view name) {
        return closure.contains(std::string{name});
    }
    LispyFunction & get_function(std::string_view name) {
        return closure.at(std::string{name});
    }
    void add_function(std::string_view name , LispyFunction && f) {
        closure.insert_or_assign(std::string{name} , std::move(f));
    }
};

static Enviroment env{};

template<class T , class V>
concept same_v_as = std::is_same_v<std::decay_t<T> , std::decay_t<V>>;

Expr eval_sepxr(SExpr & sexpr) {
    auto && [s] = sexpr;
    for(auto & e : s) eval(e);

    if(s.size() == 0) return std::move(sexpr);
    if(s.size() == 1) return std::move(s[0]);

    auto & head = s[0];
    if(std::holds_alternative<Function>(head)){
        auto & fn = env.get_function(std::get_if<Function>(&head)->name);
        return fn(env , ExprListView{s.begin() + 1 ,s.end()});
    }else 
        throw std::domain_error{"first element is not a function"};
}

void eval(Expr & e){
    e.match(overloaded{
        [&](Symbol s)       {
            if(env.contains_symbol(s.identifier)) e = Function{s.identifier};
            else throw std::invalid_argument{"unknown symbol."};
        },
        [&](SExpr & sexpr)  { e = eval_sepxr(sexpr);},
        [] (auto && _) { /* do nothing*/}
    });
}

// add builtins 

inline void check_param_nums(int sz , ExprListView & view){
    if(view.size() != sz) 
        throw std::invalid_argument{fmt::format("incorrect parameter number , expect {}" , sz)};
}

template<class T>
inline void check_param_types(ExprListView & view){
    for(auto & e : view){
        if(!std::holds_alternative<T>(e))
            throw std::domain_error{"parameters of operator should all be Number."};
    }
}

template<class BinOp>
requires 
    std::is_default_constructible_v<BinOp>  && 
    requires (BinOp b){ {b( 1 , 2)} -> std::integral ; }
Expr builtin_bin_op(Enviroment & _ , ExprListView view) {
    check_param_nums (2 , view);
    check_param_types<Number>(view);

    auto it = view.begin();
    return BinOp{}(std::get<Number>(*it)  , std::get<Number>(*(it+1)));
}

Enviroment::Enviroment() {
    add_function("+" , builtin_bin_op<std::plus<Number>>);
    add_function("-" , builtin_bin_op<std::minus<Number>>);
    add_function("*" , builtin_bin_op<std::multiplies<Number>>);
    add_function("/" , builtin_bin_op<std::divides<Number>>);
}
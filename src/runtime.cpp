#include <fmt/format.h>
#include <typeinfo>
#include <ranges>
#include <string>

#include "runtime.h"

// add builtins 

using namespace lispy;

namespace{

using ExprListView = cexpr::vector<ast::SExpr> ;

template<class T> 
const char * name () {
    if constexpr (std::is_same_v<T , ast::SExpr>) 
        return "SExpr" ;
    else if constexpr (std::is_same_v<T , ast::Atom>)
        return "Atom";
    else if constexpr (std::is_same_v<T , ast::Quote>)
        return "Quote";
    else if constexpr (std::is_same_v<T , ast::Lambda>)
        return "Procedure";
    else 
        return typeid(T).name();
}

void check_size(std::size_t sz , const ast::List & view){
    if(view->size() != sz) 
        throw runtime_error{fmt::format(
            "incorrect parameter number , got {} params , expect {} , \nin : {}" ,
             view->size() - 1, sz -1 , ast::print_sexpr(ast::SExpr{view}))};
}

template<class T>
void check_expr_type(const ast::SExpr & e) {
    if(!e.holds<T>())
        throw type_error{fmt::format(
            "{} should have type {} , but got {}" , 
            ast::print_sexpr(e) , name<T>() , name<std::remove_cvref_t<decltype(e)>>())};
}

ast::SExpr builtin_car(Closure & cls , ast::List & list){
    check_size(2 ,list.ref());
    check_expr_type<ast::Quote>(list.ref()[1]);
    
    auto & q = list.ref()[1].get_if<ast::Quote>()->ref();

    if(!q.holds<ast::List>() || q.get_if<ast::List>()->ref().empty()){
        throw type_error{fmt::format("contract fail : not a non-empty list , in {}" , ast::print_sexpr(q))};
    }

    return q.get_if<ast::List>()->ref()[0];
}

ast::SExpr builtin_cdr(Closure & cls , ast::List & list){
    check_size(2 , list.ref());
    check_expr_type<ast::Quote>(list.ref()[1]);

    auto &q = list.ref()[1].get_if<ast::Quote>()->ref();
    if(!q.holds<ast::List>() || q.get_if<ast::List>()->ref().empty()){
        throw type_error{fmt::format("contract fail : not a non-empty list , in {}" , ast::print_sexpr(q))};
    }
    cexpr::vector<ast::SExpr> ls{};
    for(auto & v : std::ranges::subrange(list.ref().begin() + 1 , list.ref().end()))
        ls.emplace_back(v);

    return ast::List{std::move(ls)};
}

std::string_view builtin_var_name(std::size_t n){
    // var_names should have static lifttime to avoid dangling ref
    static std::vector<std::string> var_names{};
    for(auto i = var_names.size() ; i <= n ; ++i ){
        var_names.emplace_back(fmt::format("@{}",i));
    }
    return var_names[n];
}

ast::Lambda make_builtin_lambda(std::string_view name , std::size_t n_bound_vars){
    ast::List body = cexpr::vector<ast::SExpr>{ast::BuiltinFn{name}};
    cexpr::vector<std::string_view> vars{};

    for(std::size_t i = 0 ; i < n_bound_vars ; ++i){
        auto var_name = builtin_var_name(i);
        body.mut().emplace_back(ast::Atom{var_name});
        vars.emplace_back(name);
    }

    return ast::Lambda {
        .var_names = std::move(vars),
        .body = ast::SExpr{body},
    };
}

}

void Runtime::init_builtins(){
    auto add_builtin = [&](std::string_view name ,Runtime::builtin_func f , std::size_t n){
        this->builtin_functions[name] = f;
        this->_global.set(name , make_builtin_lambda(name , n));
    };

    add_builtin("car" , builtin_car , 1);
    add_builtin("cdr" , builtin_cdr , 1);
}

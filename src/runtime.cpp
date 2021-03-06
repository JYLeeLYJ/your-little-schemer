#include <fmt/format.h>
#include <typeinfo>
#include <ranges>
#include <string>

#include "runtime.h"

// add builtins 

using namespace lispy;
using std::ranges::subrange;

namespace{

using ExprList = cexpr::vector<ast::SExpr> ;

template<class T> 
const char * name () {
    if constexpr (std::is_same_v<T , ast::SExpr>) 
        return "SExpr" ;
    else if constexpr (std::is_same_v<T , ast::Symbol>)
        return "Symbol";
    else if constexpr (std::is_same_v<T , ast::Quote>)
        return "Quote";
    else if constexpr (std::is_same_v<T , ast::Lambda>)
        return "Lambda";
    else if constexpr (std::is_same_v<T , ast::Boolean>)
        return "Boolean";
    else if constexpr (std::is_same_v<T , ast::Integer>)
        return "Integer";
    else if constexpr (std::is_same_v<T , ast::BuiltinFn>)
        return "BuiltinFunction";
    else 
        return typeid(T).name();
}

std::string_view current_type_name(const ast::SExpr & sexpr){
    return sexpr.match<std::string_view>(
        [](auto && v) {
            return name<std::decay_t<decltype(v)>>();
        }
    );
}

const ast::SExpr & get_param(std::size_t n , ast::List & params){
    return params.ref()[n+1];
}

ast::SExpr & get_param_mut(std::size_t n , ast::List & params){
    return params.mut()[n+1];
}

template<class T>
decltype(auto) get_param_unsafe_cast(std::size_t n , ast::List &params){
    return * (params.ref()[n + 1].get_if<T>());
}
decltype(auto) get_quote_list_unsafe(std::size_t n , ast::List &params){
    return * get_param_unsafe_cast<ast::Quote>(n , params).ref().get_if<ast::List>();
}

class schemer{
    const ast::List & _params;
    std::size_t i {1};
public:
    schemer(const ast::List & params)
    :_params(params){
        if(params->size() <= 1)
            throw runtime_error{"parameters cannot be empty."};
    }

    schemer(const ast::List & params , std::size_t n_params) 
    : _params(params){
        if(params->empty() || params->size() -1 != n_params) 
        throw runtime_error{fmt::format(
            "incorrect parameter number , got {} params , expect {} , \nin : {}" ,
             params->size() - 1, n_params , ast::print_sexpr(params))};
    }

    schemer & next(){ ++i ; return *this;}

    template<class T>
    schemer & type(){
        if( i >= _params->size()) 
            throw internal_error{fmt::format("invalid out of range schemer in {}" , ast::print_sexpr(_params.ref()[0]))};
        auto & param_i = _params.ref()[i];
        if(!param_i.holds<T>())
            throw type_error{fmt::format(
                "param {} should have type {} , but got {} \nin: {}",
                i - 1 , name<T>() , current_type_name(param_i), ast::print_sexpr(_params)
            )};
        return *this;
    }

    schemer & listp(){
        if( i >= _params->size()) 
            throw internal_error{fmt::format("invalid out of range schemer in {}" , ast::print_sexpr(_params.ref()[0]))};
        
        auto & param_i = _params.ref()[i];
        if(!param_i.holds<ast::Quote>() 
        || !param_i.get_if<ast::Quote>()->ref().holds<ast::List>())
            throw type_error{fmt::format(
                "contracts failed , {} is not a list , \nin :{} ." ,
                ast::print_sexpr(param_i) , ast::print_sexpr(_params) 
            )};
        return *this;
    }

};

ast::SExpr builtin_car(Closure & cls , ast::List & params){
    schemer(params , 1)
    .type<ast::Quote>();

    auto & q = params.ref()[1].get_if<ast::Quote>()->ref();
    if(!q.holds<ast::List>() || q.get_if<ast::List>()->ref().empty()){
        throw type_error{fmt::format("contract fail : not a non-empty list , in {}" , ast::print_sexpr(q))};
    }

    return Runtime::quote(q.get_if<ast::List>()->ref()[0]);
}

ast::SExpr builtin_cdr(Closure & cls , ast::List & params){
    schemer(params ,1 ).listp();

    auto &quote_list = get_quote_list_unsafe(0,params);
    if(quote_list.ref().empty()){
        throw type_error{fmt::format("cdr : contract fail , list is empty , in {}" , ast::print_sexpr(get_param(0,params)))};
    }

    cexpr::vector<ast::SExpr> ls{};
    for(auto & v : subrange(quote_list.ref().begin() + 1 , quote_list.ref().end()))
        ls.emplace_back(v);

    return ast::Quote{ast::List{std::move(ls)}};
}

ast::SExpr builtin_cons(Closure & cls , ast::List & params){
    schemer(params,2)
    .next()
    .listp();

    auto & lhs = get_param(0 , params);
    auto & rhs = get_quote_list_unsafe(1 , params);

    ast::List result{ExprList{lhs.holds<ast::Quote>() ? lhs.get_if<ast::Quote>()->ref() : lhs}};
    for(auto & v : rhs.ref())
        result.mut().emplace_back(v);
    return ast::Quote{std::move(result)};
}

ast::SExpr builtin_eq(Closure & cls , ast::List & params){
    schemer(params , 2);

    auto &lhs = get_param(0 , params);
    auto &rhs = get_param(1 , params);

    if(lhs.index() != rhs.index()) return false;
    return lhs.match<ast::Boolean>(overloaded{
        [&](const ast::Quote & q ){
            if(q.ref().holds_one_of<ast::Quote , ast::Lambda , ast::List>())
                return lhs == rhs;
            else 
                return q.ref() == rhs.get_if<ast::Quote>()->ref();
        },
        [&](auto & value){
            using type = std::decay_t<decltype(value)>;
            return (rhs.get<type>() == value);
        },
    });
}

ast::SExpr builtin_null(Closure & cls , ast::List & params){
    schemer(params , 1);
    if(!get_param(0 , params).holds<ast::Quote>() 
    || !get_param_unsafe_cast<ast::Quote>(0,params).ref().holds<ast::List>())
        return false;
    
    auto & ls = get_quote_list_unsafe(0 , params);
    return ls.ref().empty();
}

ast::SExpr builtin_atom(Closure & cls , ast::List & params){
    schemer(params , 1);
    auto &p = get_param(0 , params);
    return !(
        p.holds<ast::List>() ||
        (p.holds<ast::Quote>() && p.get_if<ast::Quote>()->ref().holds<ast::List>())
    );
}

ast::SExpr builtin_zero(Closure & cls , ast::List & params){
    schemer(params , 1).type<ast::Integer>();
    return 0 == get_param_unsafe_cast<ast::Integer>(0 , params);
}

ast::SExpr builtin_add1(Closure & cls , ast::List & params){
    schemer(params , 1).type<ast::Integer>();
    return get_param_unsafe_cast<ast::Integer>(0 , params) + 1;
}

ast::SExpr builtin_sub1(Closure & cls , ast::List & params){
    schemer(params , 1).type<ast::Integer>();
    return get_param_unsafe_cast<ast::Integer>(0 , params) - 1;
}

ast::SExpr builtin_add(Closure & cls , ast::List & params){
    schemer(params , 2)
    .type<ast::Integer>().next()
    .type<ast::Integer>();

    const auto & lhs = get_param_unsafe_cast<ast::Integer>(0 , params);
    const auto & rhs = get_param_unsafe_cast<ast::Integer>(0 , params);
    return lhs + rhs;
}

ast::SExpr builtin_sub(Closure & cls , ast::List & params){
    schemer(params , 2)
    .type<ast::Integer>().next()
    .type<ast::Integer>();

    const auto & lhs = get_param_unsafe_cast<ast::Integer>(0 , params);
    const auto & rhs = get_param_unsafe_cast<ast::Integer>(0 , params);
    return lhs - rhs;
}

ast::SExpr builin_div(Closure & cls ,ast::List & params){
    schemer(params , 2)
    .type<ast::Integer>().next()
    .type<ast::Integer>();

    const auto & lhs = get_param_unsafe_cast<ast::Integer>(0 , params);
    const auto & rhs = get_param_unsafe_cast<ast::Integer>(0 , params);
    if(rhs == 0) throw runtime_error{"div 0."};
    return lhs / rhs;
}

ast::SExpr builtin_mut(Closure & cls , ast::List & params){
    schemer(params , 2)
    .type<ast::Integer>().next()
    .type<ast::Integer>();

    const auto & lhs = get_param_unsafe_cast<ast::Integer>(0 , params);
    const auto & rhs = get_param_unsafe_cast<ast::Integer>(0 , params);
    return lhs * rhs;
}

ast::SExpr builtin_eval(Closure & cls , ast::List & params){
    schemer(params , 1);
    auto & p = get_param(0 , params);
    auto sexpr = p.holds<ast::Quote>()? p.get_if<ast::Quote>()->ref() : p;

    Runtime::eval_sexpr(cls , sexpr);
    return sexpr;
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
        body.mut().emplace_back(ast::Symbol{var_name});
        vars.emplace_back(var_name);
    }
    return ast::Lambda {
        .var_names = std::move(vars),
        .body = ast::SExpr{body},
    };
}

}

Runtime::Runtime() : _cls(_global) {
    init_builtins();
}

void Runtime::init_builtins(){
    auto add_builtin = [&](std::string_view name ,Runtime::builtin_func f , std::size_t n){
        this->_builtin_functions[name] = f;
        this->_global.set(name , make_builtin_lambda(name , n));
    };

    add_builtin("eval", builtin_eval, 1);

    add_builtin("car" , builtin_car , 1);
    add_builtin("cdr" , builtin_cdr , 1);
    add_builtin("cons", builtin_cons, 2);
    add_builtin("eq?" , builtin_eq  , 2);
    add_builtin("atom?",builtin_atom, 1);

    add_builtin("null?",builtin_null ,1);
    add_builtin("zero?",builtin_zero ,1);

    add_builtin("add1" , builtin_add1,1);
    add_builtin("sub1" , builtin_sub1,1);

    //set value
    _global.set("nil" , ast::List{cexpr::vector<ast::SExpr>{}});
}
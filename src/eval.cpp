#include <ranges>
#include <functional>
#include <unordered_map>
#include <fmt/format.h>

#include "ast.h"
#include "lispy.h"
#include "runtime.h"

using std::ranges::subrange;
using std::views::reverse;
using namespace std::literals;

namespace lispy {

namespace {

void eval_sexpr(Closure & cls , ast::SExpr & sexpr);
    
ast::SExpr eval_def(Closure & cls , ast::List & list){
    if(list->size() != 3 || !list.ref()[1].holds<ast::Atom>() )
        throw bad_syntax(fmt::format(
            "define : incorrect syntax in {} , expect (define symbol s-expression )" , ast::print_sexpr(ast::SExpr{list})));
    
    auto & iden = list.ref()[1];
    auto value = list.ref()[2];
    eval_sexpr(cls , value);
    // if(auto p_lambda = value.get_if<ast::Lambda>() ; p_lambda)
    //     p_lambda->name = iden.get<ast::Atom>();
    cls.global().set(iden.get<ast::Atom>() , value);
    return iden;
}

ast::SExpr eval_lambda(Closure & cls , ast::List & list){
    if(list->size() != 3 || !list.ref()[1].holds<ast::List>()) 
        throw bad_syntax(fmt::format("lambda : incorrect syntax in {} , expect (lambda (symbol *) s-expression)" , ast::print_sexpr(ast::SExpr{list}))); 

    auto & params = list.ref()[1].get<ast::List>();
    for(auto & e : params.ref())
        if(!e.holds<ast::Atom>()) 
            throw bad_syntax(fmt::format(
                "it must be a symbol denoted the parameter of lambda , in {} , expect a symbol. ",
                ast::print_sexpr(e)));
    
    ast::Lambda lambda{
        .body = list.ref()[2]
    };

    for(auto & e : params.ref()) 
        lambda.var_names.mut().emplace_back(e.get<ast::Atom>());

    return lambda;
}

std::unordered_map<std::string_view , ast::SExpr (*) (Closure & , ast::List & )>
builtin_syntax {
    {"define"sv , eval_def},
    {"lambda"sv , eval_lambda},
};

ast::SExpr invoke_function(Closure & cls , ast::List & list){
    auto & lambda = list.mut().begin()->get<ast::Lambda>();
    auto n_except = lambda.var_names->size() - lambda.bounded->size();
    // too many arguments
    if(n_except < list->size())
        throw runtime_error(fmt::format(
            "argument size mismatch in {}, expect {} , got {} , ",
            ast::print_sexpr(list) , n_except , list->size()));

    for(auto & e : subrange(list.ref().begin() + 1 , list.ref().end()))
        lambda.bounded.mut().emplace_back(e);
    
    //currying 
    if(lambda.var_names->size() - lambda.bounded->size() > 0){
        return lambda;
    }
    // eval
    else{
        Environment tmp_env{};
        for(std::size_t i = 0 ; i < lambda.var_names->size() ; ++i){
            tmp_env.set(lambda.var_names.ref()[i] , lambda.bounded.ref()[i]);
        }
        auto auto_pop = cls.push(tmp_env);
        eval_sexpr(cls , lambda.body.mut());
        return lambda.body.ref();
    }
}

ast::SExpr eval_list(Closure & cls , ast::List & list){
    if(list->size() == 0) 
        throw runtime_error("missing procedure expression , given emtpy ().");

    auto & head = * (list->begin());
    //1. statement 
    if(auto iden = head.get_if<ast::Atom>() ; iden && builtin_syntax.contains(*iden)){
        return builtin_syntax.at(*iden) (cls , list);
    }
    //2. procedure
    //TODO : avoid list mut
    for(auto & e : list.mut()) eval_sexpr(cls ,e);
    if(head.holds<ast::Lambda>()){
        return invoke_function(cls ,list);
    }else if(head.holds<ast::BuiltinFn>()) {
        auto builtin = Runtime::get_builtin_func(head.get_if<ast::BuiltinFn>()->name);
        return builtin(cls , list);
    }
    //3. false
    else 
        throw runtime_error(fmt::format("not a procedure , given {}" , ast::print_sexpr(head)));
}

void eval_sexpr(Closure & cls , ast::SExpr & sexpr){
    sexpr.match(overloaded{
        [&] (ast::List & l)     { sexpr = eval_list(cls , l); },
        [&] (ast::Atom & name)  {   
            if(auto v = cls.find(name) ; v) sexpr = *v;
            else throw runtime_error(fmt::format("undefined {}" , name));
        },
        []  (auto && _)         {   /* do nothing */},
    });
}

}

std::optional<ast::SExpr> Closure::find(std::string_view name) noexcept{
    for(Environment & env : _cls | reverse){
        if(auto v = env.get(name) ; v) return v;
    }
    return std::nullopt;
}

std::string Runtime::eval(std::string_view input){
    auto result = ast::parse(input);
    if(!result) throw parse_error("parse error.");
    auto & rt = Runtime::instance();
    //TODO : exception safety
    if(result->holds<ast::List>() || result->holds<ast::Atom>())
        eval_sexpr(rt._cls , *result);
    return print_sexpr(*result);
}

}
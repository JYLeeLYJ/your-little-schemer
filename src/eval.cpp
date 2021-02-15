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

ast::SExpr eval_def(Closure & cls , ast::List & list){
    if(list->size() != 3 || !list.ref()[1].holds<ast::Symbol>() )
        throw bad_syntax(fmt::format(
            "define : bad syntax in {}." , ast::print_sexpr(ast::SExpr{list})));
    
    auto & iden = list.ref()[1];
    auto value = list.ref()[2];
    Runtime::eval_sexpr(cls , value);
    cls.global().set(iden.get<ast::Symbol>() , value);
    return iden;
}

ast::SExpr eval_lambda(Closure & cls , ast::List & list){
    if(list->size() != 3 || !list.ref()[1].holds<ast::List>()) 
        throw bad_syntax(fmt::format("lambda : bad syntax , in {} . " , ast::print_sexpr(ast::SExpr{list}))); 

    auto & params = list.ref()[1].get<ast::List>();
    for(auto & e : params.ref())
        if(!e.holds<ast::Symbol>()) 
            throw bad_syntax(fmt::format("lambda : bad syntax , expect a symbol , in {} . ",ast::print_sexpr(e)));
    
    ast::Lambda lambda{.body = list.ref()[2]};

    for(auto & e : params.ref()) 
        lambda.var_names.mut().emplace_back(e.get<ast::Symbol>());

    return lambda;
}

ast::SExpr eval_and(Closure & cls , ast::List & params){
    if(params->size() <= 1)
        throw runtime_error{"parameters cannot be empty."};

    auto ref_result = std::ref(*params.mut().begin());
    for(auto & conds: subrange(params.mut().begin() + 1 , params.mut().end())){
        Runtime::eval_sexpr(cls , conds);
        if(conds == ast::Boolean{false}) return conds;
        else ref_result = std::ref(conds);
    }
    return ref_result.get();
}

ast::SExpr eval_or(Closure & cls , ast::List & params){
    if(params->size() <= 1)
            throw runtime_error{"parameters cannot be empty."};

    auto ref_result = std::ref(*params.mut().begin());
    for(auto & conds : subrange(params.mut().begin() + 1 , params.mut().end())){
        Runtime::eval_sexpr(cls , conds);
        if(conds == ast::Boolean{true}) return conds;
        else ref_result = std::ref(conds);
    }
    return ref_result;
}

ast::SExpr eval_cond(Closure & cls , ast::List & list){
    if(list->size() < 3 )
        throw bad_syntax(fmt::format("cond : bad syntax , in {} ." , ast::print_sexpr(list)));

    auto branchs = subrange(list.ref().begin() + 1 , list.ref().end());
    if(branchs.size() < 2)
        throw bad_syntax(fmt::format("cond : bad syntax , in {} ." , ast::print_sexpr(list)));

    for(auto & e : branchs) 
        if(!e.holds<ast::List>() || e.get_if<ast::List>()->ref().size() != 2)
            throw bad_syntax(fmt::format("cond : bad syntax , in {}" , ast::print_sexpr(e)));

    auto conds = subrange(branchs.begin() , branchs.end() - 1);
    auto else_var = *(branchs.end() - 1);
    if(else_var.get_if<ast::List>()->ref()[0] != ast::Symbol{"else"})
        throw bad_syntax(fmt::format("cond : bad syntax in {} , expect (else S-Expression)" , ast::print_sexpr(list)));
    
    for(auto & e : conds){
        auto & pair = *e.get_if<ast::List>();
        auto cond = pair.ref()[0];
        Runtime::eval_sexpr(cls , cond);
        if(cond == ast::Boolean{true}){
            auto value = pair.ref()[1];
            Runtime::eval_sexpr(cls , value);
            return value;
        }
    }
    auto var = else_var.get_if<ast::List>()->ref()[1];
    Runtime::eval_sexpr(cls , var);
    return var;
}

std::unordered_map builtin_syntax {
    std::pair
    {"define"sv , eval_def},
    {"lambda"sv , eval_lambda},
    {"cond"sv   , eval_cond},
    {"and"sv    , eval_and},
    {"or"sv     , eval_or} ,
};

ast::SExpr invoke_function(Closure & cls , ast::List & list){
    auto & lambda = list.mut().begin()->get<ast::Lambda>();
    auto n_except = lambda.var_names->size() - lambda.bounded->size();
    // too many arguments
    if(n_except < list->size() -1 )
        throw runtime_error(fmt::format(
            "argument size mismatch in {}, expect {} , got {} , ",
            ast::print_sexpr(list) , n_except , list->size()));

    for(auto & e : subrange(list.ref().begin() + 1 , list.ref().end()))
        lambda.bounded.mut().emplace_back(e);
    
    //currying 
    if(lambda.var_names->size() > lambda.bounded->size()){
        return lambda;
    }
    //invoke
    else{
        Environment tmp_env{};
        for(std::size_t i = 0 ; i < lambda.var_names->size() ; ++i){
            tmp_env.set(lambda.var_names.ref()[i] , lambda.bounded.ref()[i]);
        }
        auto auto_pop = cls.push(tmp_env);
        Runtime::eval_sexpr(cls , lambda.body.mut());
        return lambda.body.ref();
    }
}

ast::SExpr eval_list(Closure & cls , ast::List & list){
    if(list->size() == 0) 
        throw runtime_error("missing procedure expression , given emtpy ().");

    //1. statement 
    {
        auto & head = * (list->begin());
        if(auto iden = head.get_if<ast::Symbol>() ; iden && builtin_syntax.contains(*iden)){
            return builtin_syntax.at(*iden) (cls , list);
        }
    }
    //2. procedure
    //TODO : avoid list mut
    for(auto & e : list.mut()) Runtime::eval_sexpr(cls ,e);
    auto & head = * (list->begin());
    if(head.holds<ast::Lambda>()){
        return invoke_function(cls ,list);
    }else if(head.holds<ast::BuiltinFn>()) {
        auto builtin = Runtime::get_builtin_func(head.get_if<ast::BuiltinFn>()->name);
        return builtin(cls , list);
    }
    //3. false
    else 
        throw runtime_error(fmt::format(
            "not a procedure , given {} , \nin {}" , 
            ast::print_sexpr(head),ast::print_sexpr(list))
        );
}

}

bool is_need_quote(const ast::SExpr & e ){
    return (e.holds<ast::Quote>() || e.holds<ast::List>() || e.holds<ast::Symbol>());
}

ast::SExpr Runtime::quote(ast::SExpr e){
    return is_need_quote(e) ? ast::SExpr{ast::Quote{std::move(e)}} : e;
}

void Runtime::eval_sexpr(Closure & cls , ast::SExpr & sexpr){
    sexpr.match(overloaded{
        [&] (ast::List & l)     { 
            sexpr = eval_list(cls , l); 
        },
        [&] (ast::Symbol & name){   
            if(auto v = cls.find(name) ; v) sexpr = *v;
            else throw runtime_error(fmt::format("undefined {}" , name));
        },
        [&] (ast::Quote & q)    {
            if(! is_need_quote(q.ref())) sexpr = q.ref();
        },
        []  (auto && _)         {   /* do nothing */},
    });
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
    if(result->holds<ast::List>() || result->holds<ast::Symbol>() || result->holds<ast::Quote>())
        Runtime::eval_sexpr(rt._cls , *result);
    return print_sexpr(*result);
}

}
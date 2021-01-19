#pragma once
#include <functional>
#include <unordered_map>
#include "lispy.h"

namespace lispy{

struct ExprListView {
    ExprList::const_iterator _beg , _end;
    
    auto begin()const   {return _beg;}
    auto end()  const   {return _end;}

    bool empty() const  {return _end == _beg;}
    auto size() const   {return _end - _beg;}
};

class Environment;
using LispyFunction = std::function<Expr (Environment &, ExprListView)>;
using Closure = std::unordered_map<std::string , std::variant<Expr ,LispyFunction> >;

class Environment{
    Closure closure;
    using key_type = std::string;
protected:
    Environment() ;
public:
    bool contains_symbol(std::string_view name) {
        return closure.contains(std::string{name});
    }
    LispyFunction * get_function(std::string_view name) {
        return std::get_if<LispyFunction>(& closure.at(std::string{name}));
    }
    Expr * get_var(std::string_view name){
        return std::get_if<Expr>(& closure.at(std::string{name}));
    }
    void add_function(std::string_view name , LispyFunction f) {
        closure.insert_or_assign(std::string{name} , std::move(f));
    }
    void add_variable(std::string_view name , Expr e){
        closure.insert_or_assign(std::string{name} , std::move(e));
    }
};

}

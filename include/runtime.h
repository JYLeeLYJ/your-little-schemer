#pragma once
#include <functional>
#include <optional>
#include <unordered_map> 
#include <vector>
#include <memory>

#include "lispy.h"
#include "ast.h"

namespace lispy{

    //scope
    class Environment{
        std::unordered_map<std::string_view , ast::SExpr> env{};
    public:
        bool contains(std::string_view s) const {
            return env.contains(s);
        } 
        auto get(std::string_view s) -> std::optional<ast::SExpr> const {
            auto it = env.find(s);
            return it == env.end() ? std::nullopt : std::optional{it->second};
        }
        void set(std::string_view s , ast::SExpr sexpr){
            env.emplace(s, std::move(sexpr));
        }
    };

    //Closure
    class Closure {
        std::vector<std::reference_wrapper<Environment>> _cls{};
    public:
        struct pop_guard{
            Closure & rcls;
            pop_guard(Closure & ref) noexcept : rcls(ref) {}
            pop_guard(const pop_guard & ) = delete;
            pop_guard(pop_guard && ) = delete;
            ~pop_guard() { rcls.pop();}
        };
    public :
        Closure(Environment & global) 
        : _cls{std::ref(global)} {}

        [[nodiscard]]
        std::optional<ast::SExpr> find(std::string_view name) noexcept;

        [[nodiscard]]
        pop_guard push(Environment & env){
            _cls.emplace_back(std::ref(env));    
            return *this;
        }

        Environment & global(){
            return * _cls.begin();
        }
    protected:
        void pop() noexcept{
            if(!_cls.empty()) _cls.pop_back();
        }
    };

    //variable type 
    class Runtime {
        Runtime() : _cls(_global) { init_builtins(); }
    public:
        using builtin_func = ast::SExpr (*) (Closure & , ast::List & );

        static Runtime & instance() {
            static Runtime rt{};   return rt;
        }
        
        static builtin_func get_builtin_func(std::string_view name){
            return instance().builtin_functions.at(name);
        }

        static bool is_builtin_function(std::string_view name){
            return instance().builtin_functions.contains(name);
        }

        static std::string eval(std::string_view input);

    private:
        void init_builtins();
    private:
        // Environment _global_tmp;
        Environment _global{};
        Closure _cls;
        std::unordered_map<std::string_view , builtin_func> builtin_functions{};
    };

}





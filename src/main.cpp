#include <iostream>
#include <cstdlib>
#include <string_view>
#include <memory>
#include <functional>

#include <fmt/format.h>
#include <fmt/printf.h>
#include <editline/history.h>
#include <editline/readline.h>

#include <variant>
#include <stdexcept>
#include <numeric>
#include "parsec.h"

struct repl_io{
    auto operator >> (std::function<std::string(std::string_view)> && f){
        return [f = std::move(f)]{
            while(true){
                std::unique_ptr<char , decltype(&free)> str_guard(readline("lispy>") , free);
                std::string_view input{str_guard.get()};
                add_history(input.data());
                if(input == ":q") break;
                try{
                    auto result = f(input);
                    fmt::print("{}\n",result);
                }catch(const std::exception & e){
                    fmt::print("{}\n",e.what());
                }
            }
        };
    }
};

using namespace pscpp;

constexpr int to_int(std::string_view nums){
    auto v = nums | std::ranges::views::transform([](char ch)->int {return ch - '0';}) ;
    return std::accumulate(v.begin() , v.end(), 0 , [](int init , int i ){return init * 10 + i;});
}

constexpr int eval_lispy(char op , cvector<int> ints){
    switch (op){
    case '+': return std::accumulate(ints.begin() , ints.end() , 0 , std::plus<int>{});
    case '-': return std::accumulate(ints.begin() , ints.end() , 0 , std::minus<int>{});
    case '*': return std::accumulate(ints.begin() , ints.end() , 1 , std::multiplies<int>{});
    case '/':
        if(ints.size()!=2) throw std::invalid_argument{R"(incorrect argument for '/' operation .)"};
        return ints[0] / ints[1];
    default : throw std::domain_error{"unexcept operator."};
    }
}

auto natural = fmap(to_int , chars(digit));
auto negative = fmap(std::negate<int>{} , ('-'_char >> natural)) ;

auto sub_expr(parser_string str)->parser_result<int>; 

auto 
numbers     = negative | natural;
auto 
opt         = spaces >> one_of("+-*/") << spaces;
auto 
expr        = spaces >> (numbers | sub_expr) << spaces;
auto 
lispy       = fmap(eval_lispy , opt , many(expr));
auto 
_sub_expr   = '('_char >> lispy << ')'_char;

auto sub_expr(parser_string str)->parser_result<int>{
    return _sub_expr(str);
}

auto polish = (expr | lispy) << eof ;

auto main_loop = repl_io{} >> [](std::string_view input){

    auto result = polish(input);
    if(!result) 
        return fmt::format("invalid input format");
    else 
        return fmt::format(" {} = {}" , input , result->first);

};

int main(){
    main_loop();
}

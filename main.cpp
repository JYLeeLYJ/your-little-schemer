#include <iostream>
#include <cstdlib>
#include <string_view>
#include <memory>
#include <functional>

#include <fmt/format.h>
#include <fmt/printf.h>
#include <editline/history.h>
#include <editline/readline.h>

struct repl_io{
    auto operator >>= (std::function<std::string(std::string_view)> && f){
        return [f = std::move(f)]{
            while(true){
                std::unique_ptr<char , decltype(&free)> str_guard(readline("lispy>") , free);
                std::string_view input{str_guard.get()};
                add_history(input.data());
                if(input == ":q") break;
                auto result = f(input);
                fmt::print("{}\n",result);
            }
        };
    }
};

auto main_loop = repl_io{} >>= [](std::string_view input){
    return fmt::format("input = {}" ,  input);
};

int main(){
    main_loop();
}

#include <cstdlib>
#include <string_view>
#include <memory>
#include <functional>

#include <fmt/format.h>
#include <fmt/printf.h>
#include <editline/history.h>
#include <editline/readline.h>

#include "lispy.h"

void put_version_info(){
    fmt::print("\n");
    fmt::print("Lispy Version 0.0.0.0.1\n");
    fmt::print("Press Ctrl+c to Exit\n\n");
}

struct repl_io{
    auto operator >> (std::function<std::string(std::string_view)> && f){
        return [f = std::move(f)]{
            put_version_info();
            while(true){
                using RAII_GuardString = std::unique_ptr<char , decltype(&free)>;
                RAII_GuardString str_guard{readline("lispy>") , free};
                std::string_view input{str_guard.get()};
                add_history(input.data());
                if(input == ":q") break;
                if(input.empty()) continue;
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

auto main_loop = repl_io{} >> [](std::string_view input){

    auto result = parse_lispy(input);
    if(!result) 
        return fmt::format("invalid input format");

    eval(*result);
    return print_expr(*result);
};

int main(){
    main_loop();
}
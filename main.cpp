#include <cstdlib>
#include <string_view>
#include <memory>
#include <functional>

#include <fmt/format.h>
#include <fmt/printf.h>
#include <editline/history.h>
#include <editline/readline.h>

#include "lispy.h"

struct repl_io{
    auto operator >> (std::function<std::string(std::string_view)> && f){
        return [f = std::move(f)]{
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

    auto result = parse_polish(input);
    if(!result) 
        return fmt::format("invalid input format");
    else 
        return fmt::format(" {} = {}" , input , *result);

};

int main(){
    main_loop();
}
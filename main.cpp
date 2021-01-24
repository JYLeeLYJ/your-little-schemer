#include <cstdlib>
#include <string_view>
#include <memory>
#include <vector>
#include <functional>

#include <fmt/format.h>
#include <fmt/printf.h>
#include <editline/history.h>
#include <editline/readline.h>

#include "lispy.h"
#include "runtime.h"

void put_version_info(){
    fmt::print("\n");
    fmt::print("Lispp Version 0.0.2\n");
    fmt::print("Press Ctrl+c to Exit\n\n");
}

using RAII_GuardString = std::unique_ptr<char , decltype(&free)>;
std::vector<RAII_GuardString> history_codes{};

struct repl_io{
    auto operator >> (std::function<std::string(std::string_view)> && f){
        return [f = std::move(f)]{
            put_version_info();
            while(true){
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
                history_codes.emplace_back(std::move(str_guard));
            }
        };
    }
};

auto main_loop = repl_io{} >> lispy::Runtime::eval ;

int main(){
    main_loop();
}
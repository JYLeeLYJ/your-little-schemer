#pragma once
#include "combinator.h"

auto digit (parser_string str) -> parser_result<char>{
    static const auto _digit = range('0' , '9');
    return _digit(str);
}

auto lower (parser_string str) -> parser_result<char>{
    static const auto _lower = range('a' , 'z');
    return _lower(str);
}

auto upper (parser_string str) -> parser_result<char>{
    static const auto _upper = range('A' , 'Z');
    return _upper(str);
}

auto letter (parser_string str) -> parser_result<char>{
    static const auto _letter = parser(upper) | parser(lower);
    return _letter(str);
}

auto alphanum(parser_string str)-> parser_result<char>{
    static const auto _alphanum = parser(letter) | parser(digit);
    return _alphanum(str);
}

// auto word(std::string_view str) -> parser_result<std::string_view> {
//     using namespace std::literals;
    
//     auto neword = 
//     letter  >>= [=](char c)                 {return 
//     word    >>= [=](std::string_view res)   {return 
//         result(str.substr(0 ,res.size() + 1));  // instead of concat
//     };};

//     return plus(neword , result(""sv))(str);
// }

auto word(std::string_view str) -> parser_result<std::string_view> {
    using namespace std::literals;
    static const auto neword = 
        (parser(letter) + parser(word)) 
        >>= [=](auto && tp){
            auto &&[_ , xs] = tp.tp;
            return result(str.substr(0,xs.size() + 1));
        };
    static const auto _word = (neword | result(""sv));
    return _word(str);
}

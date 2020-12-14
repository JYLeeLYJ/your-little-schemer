#pragma once

#include <string>
#include <concepts>

#include "utils.h"
#include "combinator.h"

inline parser_t<char> digit = range('0' , '9');

inline parser_t<char> upper = range('A' , 'Z');

inline parser_t<char> lower = range('a' , 'z');

inline parser_t<char> letter = upper | lower ;

inline parser_t<char> alphanum = letter | digit ;

inline parser_t<std::string> word = 
    fix([](auto && word) -> parser_t<std::string>{
        using namespace std::literals;
        return plus(
            ((letter + word) >>= [](const product_type<char , std::string> & tp){
                auto &&[ch , xs] = tp.tp;
                return result(ch + xs);
            }) 
            , result(std::string{})
        );
    });

template<std::integral T>
inline parser_t<T> natural = many1(digit) 
    >>= [](auto && forward_list) {
        return result(char_list_to_integer<T>(forward_list));
    };

template<std::integral T>
inline parser_t<T> negative = (onechar('-') + natural<T>) 
    >>= [](auto && tp){ 
        return result(-std::get<1>(tp.tp)); 
    };

template<std::integral T>
inline parser_t<T> integer = natural<T> | negative<T> ;

//parser [int]
inline auto ints = bracket(
        onechar('[') , 
        sepby1(integer<int> , onechar(',')) , 
        onechar(']')
    );
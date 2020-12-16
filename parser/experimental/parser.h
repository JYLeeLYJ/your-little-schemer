#pragma once

#include <string>
#include <concepts>

#include "utils.h"
#include "combinator.h"
//parser char
auto item (parser_string str) -> parser_result<char>{
    if(str.empty()) return {};
    else            return std::pair{str[0] , str.substr(1)};
}

// more combinators

// (char -> bool) -> parser char
template<std::predicate<char> Pred>
parseable auto satisfy(Pred && predicate) {
    return item 
    >>= [=](char x)-> parser_t<char>{
        if (predicate(x)) return result(x);  
        else return zero<char>;
    };
}

// a -> a -> parser a
template<class T>
parseable auto range(T l , T h){
    return satisfy([=](const T & t){ return t >= l && t <= h;});
}

//high-level combinators

//char -> parser char
parseable auto onechar(char ch){
    return satisfy([=](char x)->bool{return x == ch;});
}

//string -> parser string
auto string(std::string_view word) -> parser_t<std::string_view>{
    using namespace std::literals;
    if(word.empty()) return result(""sv);
    else return 
        (onechar(word[0]) + string(word.substr(1))) 
        >>= [=](auto && _) {return result(word);};
};

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
inline parser_t<T> negative = (onechar('-') >> natural<T>) 
    >>= [](T && t) {return result(-t) ;};

template<std::integral T>
inline parser_t<T> integer = natural<T> | negative<T> ;

//parser [int]
inline auto ints = bracket(
        onechar('[') , 
        sepby1(integer<int> , onechar(',')) , 
        onechar(']')
    );
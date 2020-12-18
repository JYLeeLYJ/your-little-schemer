#pragma once

#include <string>
#include <concepts>

#include "utils.h"
#include "combinator.h"

//parser char
inline auto item (parser_string str) -> parser_result<char>{
    if(str.empty()) return {};
    else            return std::pair{str[0] , str.substr(1)};
}

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
inline parseable auto range(char l , char h){
    return satisfy([=](char c){ return c >= l && c <= h;});
}

//[char] -> parser [char]
inline parseable auto oneof(std::string_view chs){
    return satisfy([=](char ch){ return chs.find(ch) != chs.npos;});
}

//char -> parser char
inline parseable auto onechar(char ch){
    return satisfy([=](char x)->bool{return x == ch;});
}

//string -> parser string
inline auto string(std::string_view word) -> parser_t<std::string_view>{
    using namespace std::literals;
    if(word.empty()) return result(""sv);
    else return 
        (onechar(word[0]) + string(word.substr(1))) 
        >>= [=](auto && _) {return result(word);};
};

inline parseable auto operator ""_char (char ch) {
    return onechar(ch);
}

inline parseable auto operator ""_string(const char * word){
    return string(word);
}

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
inline parser_t<T> natural = lift(char_list_to_integer<int>) * many1(digit);

template<std::integral T>
inline parser_t<T> negative = lift(std::negate<T>{}) * ('-'_char >> natural<T>) ;

template<std::integral T>
inline parser_t<T> integer = natural<T> | negative<T> ;

//parser [int]
template<std::integral T>
inline auto ints = '['_char >> sepby1(integer<T> , ','_char ) << ']'_char;

//Lexial 

inline auto newline = '\n'_char;

inline auto is_space = ' '_char | '\n'_char | '\t'_char;

inline auto spaces = many(is_space);
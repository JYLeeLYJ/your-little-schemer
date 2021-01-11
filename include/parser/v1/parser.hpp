#pragma once

#include "combinator.hpp"

namespace pscpp{

constexpr parser_result<char> item(parser_string str){
    if(str.empty()) return {};
    else return std::pair{str[0] , str.substr(1)};
}

constexpr parser_result<none_t> eof(parser_string str){
    if(!str.empty()) return {};
    else return std::pair{none_t{} , str};
}

template<std::predicate<char> Pred>
constexpr Parser auto satisfy(Pred p){
    return [=](parser_string str) -> parser_result<char>{
        auto ch_res = item(str);
        if(ch_res && p(ch_res->first)) return ch_res;
        else return {};
    };
}

constexpr Parser auto range(char l , char h){
    return satisfy([=](char c){return l<=c && c<=h;});
}

constexpr Parser auto onechar(char ch){
    return satisfy([=](char c){return ch == c;});
}

constexpr Parser auto one_of(std::string_view ch_list){
    return satisfy([=](char c){return ch_list.find(c) != ch_list.npos;});
}

constexpr Parser auto string(std::string_view s){
    return [=](parser_string str) -> parser_result<std::string_view>{
        if(str.starts_with(s)) return std::pair{s , str.substr(s.size())};
        else return {};
    };
}

template<Parser P>
constexpr Parser auto chars(P && p){
    return [=](parser_string str) -> parser_result<std::string_view>{
        auto res = foldl_parse(str , p , 0 , [](int i , char c){return i+1;});
        if(!res) return {};
        auto & [len , state] = *res;
        return std::pair{str.substr(0,len) , state};
    };
}

constexpr auto operator ""_char (char c){
    return onechar(c);
}

constexpr auto operator ""_str (const char * s , std::size_t len){
    return string(std::string_view{s,len});
}

inline constexpr auto digit = range('0' , '9');

inline constexpr auto upper = range('A' , 'Z');

inline constexpr auto lower = range('a' , 'z');

inline constexpr auto letter = upper | lower ;

inline constexpr auto alphanum = letter | digit ;

inline constexpr auto newline = '\n'_char;

inline constexpr auto spaces = many(skip(' '_char | '\n'_char | '\t'_char));

inline constexpr auto spaces1= many1(skip(one_of(" \n\t")));

};


#pragma once

#include <concepts>
#include "combinator.hpp"

constexpr parser_result<char> item(parser_string str){
    if(str.empty()) return {};
    else return std::pair{str[0] , str.substr(1)};
}

template<std::predicate<char> Pred>
constexpr Parser auto satisfy(Pred p){
    return [=](parser_string str) -> parser_result<char>{
        auto ch_res = item(str);
        if(!ch_res || !(p(ch_res->first))) return {};
        else return ch_res;
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

constexpr auto operator ""_char (char c){
    return onechar(c);
}

constexpr auto operator ""_string(const char * s){
    return string(s);
}

inline constexpr auto digit = range('0' , '9');

inline constexpr auto upper = range('A' , 'Z');

inline constexpr auto lower = range('a' , 'z');

inline constexpr auto letter = upper | lower ;

inline constexpr auto alphanum = letter | digit ;

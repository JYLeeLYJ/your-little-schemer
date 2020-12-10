#pragma once 

#include <string_view>
#include <optional>
#include <functional>
#include <type_traits>
#include <concepts>
#include <string>

#include "utils.h"

using parser_string = std::string_view;

template<class T>
using parser_result = std::optional<std::pair<T , parser_string>>;

template<class T>
using result_type = std::result_of_t<T(std::string_view)>;

template<class T>
struct is_parser_result : std::false_type{};

template<class T>
struct is_parser_result<parser_result<T>>: std::true_type{};

template<class T >
concept parseable = is_parser_result<result_type<T>>{} == true;

template<parseable T>
struct parser_traits:std::true_type{
    using type = result_type<T>::value_type::first_type;
};

template<class T1 , class T2>
concept same_parser = 
    parseable<T1> && 
    parseable<T2> && 
    std::is_same_v<typename parser_traits<T1>::type ,typename parser_traits<T2>::type>;

//three primitive parser

//a -> parser a
template<class T>
constexpr auto result(T && t){
    return [t = std::forward<T>(t)](parser_string str)->parser_result<std::remove_cvref_t<T>>{
        return std::pair{t , str};
    };
}

//parser a
template<class T>
constexpr auto zero = [](parser_string str)->parser_result<T> {return std::nullopt;};

//parser char
constexpr auto item = [](parser_string str)->parser_result<char> {
    if(str.empty()) return std::nullopt ;
    else return std::pair{str[0] , str.substr(1)};
};

//basic combinators

//parser a -> parser b -> parser (a,b)
template<parseable P1 , parseable P2>
constexpr auto seq(P1 && p1 , P2 && p2){
    return [p1 = std::forward<P1>(p1) , p2 = std::forward<P2>(p2)](parser_string str)->auto{
        if      (auto r1 = p1(str);!r1) return std::nullopt;
        else if (auto r2 = p2(r1.second);!r2) return std::nullopt;
        else    return  std::pair{cat_tuple(r1.first , r2.first) , r2.second};
    };
}

//parser a -> (a->parser b) -> parser b
template<parseable P1 , class F>
constexpr auto bind(P1 && p1 , F && f) requires std::is_invocable_v<F , typename parser_traits<P1>::type> {
    using T1 = parser_traits<P1>::type;                             //a
    using T2 = parser_traits<std::invoke_result_t<F , T1>>::type;   //b
    return [p1 = std::forward<P1>(p1) , f = std::forward<F>(f)](parser_string str)->parser_result<T2>{
        auto r1 = p1(str);
        if(r1 == std::nullopt ) 
            return {};
        else 
            return f(std::move(r1.value().first))(r1.value().second);
    };
}

//parser a -> parser a -> parser a
template<parseable P1 , parseable P2>
constexpr auto plus(P1 && p1 , P2 && p2) requires same_parser<P1 ,P2> {
    using T1 = typename parser_traits<P1>::type;
    return [p1 = std::forward<P1>(p1) , p2 = std::forward<P2>(p2)](parser_string str)->parser_result<T1>{
        if(auto r1 = p1(str); r1) return r1;
        if(auto r2 = p2(str); r2) return r2;
        return {};
    };
};

template<parseable P1 , class F>
constexpr auto operator >>= (P1 && p1 , F && f){
    return bind(std::forward<P1>(p1) , std::forward<F>(f));
}

//more combinators

// (char -> bool) -> parser char
template<std::predicate<char> F>
constexpr auto satisfy(F && predicate) {
    return item 
    >>= [p = std::forward<F>(predicate)](char x)-> std::function<parser_result<char>(parser_string)>{
        if (p(x)) return std::function{result(x)};  //TODO : type erase to pass compilation
        else return std::function{zero<char>};
    };
}

constexpr auto onechar(char c){
    return satisfy([=](char ch)->bool{return c == ch;});
}

template<class T>
constexpr auto in_range(T a , T b){
    return satisfy([=](std::same_as<T> auto x)->bool{return x >= a && x < b; });
}

constexpr auto digit = in_range('0' , '9');

constexpr auto lower = in_range('a' , 'z');

constexpr auto upper = in_range('A' , 'Z');

constexpr auto letter = plus(lower , upper);

constexpr auto alphanum = plus(letter , digit);

//foward declearation to avoid recursive combinator usage
constexpr auto word(std::string_view str) -> parser_result<std::string_view>;

constexpr auto neword = 
    letter >>=  [=](char c){return 
    word >>=    [=](std::string_view res){return 
        result(str.substr(0 ,res.size() + 1));  // instead of string concat operation
    };};

constexpr auto word(std::string_view str) -> parser_result<std::string_view> {
    using namespace std::literals;
    return plus(neword , result(""sv))(str);
}

constexpr auto string =  [](std::string_view word){
    return [=](std::string_view str) -> parser_result<std::string_view>{
        if(str.length() < word.length() || !str.starts_with(word))   
            return {};
        else 
            return std::pair{word , str.substr(word.length())};
    };
};

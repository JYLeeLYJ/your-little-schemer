#pragma once 

#include <string_view>
#include <optional>
#include <functional>
#include <type_traits>
#include <concepts>

//TODO : inline constexpr

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

template<class T1 , class T2>
concept add = requires (T1 t1 , T2 t2){ 
    t1 + t2;
};

template<parseable T>
struct parser_traits:std::true_type{
    using type = result_type<T>::value_type::first_type;
};

template<class T>
struct parser_t{
    std::function<parser_result<T>(parser_string)> _f;
    template<parseable F>
    parser_t(F && f) noexcept
    : _f(std::move(f)){}
};

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
    using T1 = typename parser_traits<P1>::type;
    using T2 = typename parser_traits<P2>::type;

    return [p1 = std::forward<P1>(p1) , p2 = std::forward<P2>(p2)](parser_string str)->parser_result<std::tuple<T1,T2>>{
        auto r1 = p1(str);
        if(!r1) return std::nullopt;
        auto r2 = p2(r1.second);
        if(!r2) return std::nullopt;
        return std::pair{std::tuple{r1.first , r2.first} , r2.second};
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

template<parseable P1 , class F>
constexpr auto operator >>= (P1 && p1 , F && f){
    return bind(std::forward<P1>(p1) , std::forward<F>(f));
}

template<parseable P1 , parseable P2>
constexpr auto operator + (P1 && p1 , P2 && p2){    
    return seq(std::forward<P1>(p1) , std::forward<P2>(p2));
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
constexpr auto range(T a , T b){
    return satisfy([=](std::same_as<T> auto x)->bool{return x >= a && x < b; });
}

constexpr auto digit = range('0' , '9');

constexpr auto lower = range('a' , 'z');

constexpr auto upper = range('A' , 'Z');
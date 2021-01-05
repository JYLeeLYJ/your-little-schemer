#pragma once

#include <ranges>

#include "types.hpp"
#include "parser_utils.hpp"

#include <iostream>

namespace pscpp{

//a -> parser a
template<class T>
constexpr auto result(T && t){
    return [t= std::forward<T>(t)](parser_string str) -> parser_result<T>{
        return std::pair{t , str};
    };
}

template<class T>
requires std::is_default_constructible_v<T>
constexpr auto result_default(parser_string str) -> parser_result<T>{
    return std::pair{T{} , str};
}; 

//parser a
template<class T>
constexpr auto zero(parser_string ) -> parser_result<T> {
    return {};
}


//parser a -> (a -> b) -> parser b
template<Callable F , Parser ...Ps>
requires std::invocable<F , typename parser_traits<Ps>::type ...>
constexpr auto fmap(F && f , Ps && ...ps) {
    using R = std::invoke_result_t<F , typename parser_traits<Ps>::type ...>;
    return [=](parser_string str) -> parser_result<R> {
        auto res = chain_parse(str , ps ...);
        if(!res) return {};
        return std::pair{std::apply(f , res->first) , res->second};
    };
}

//(a->b) -> parser (a->b)
template<Callable F>
constexpr Parser auto lift(F &&  f){
    return result(std::forward<F>(f));
}

//parser a -> (a -> parser b) -> parser b
template<Parser P , class F>
requires std::invocable<F , typename parser_traits<P>::type>
constexpr Parser auto bind(P && p , F && f){
    using PT= std::invoke_result_t<F , typename parser_traits<P>::type>;
    using T = typename parser_traits<PT>::type;
    return [=](parser_string str)->parser_result<T>{
        auto r1 = p(str);
        if(!r1) return {};
        else return f(std::move(r1->first))(r1->second);
    };
}

//parser a -> parser a -> parser a
template<Parser P1, Parser P2>
requires std::is_same_v<typename parser_traits<P1>::type , typename parser_traits<P2>::type>
constexpr Parser auto plus(P1 && p1 , P2 && p2){
    using R = typename parser_traits<P1>::type;
    return [=](parser_string str)->parser_result<R>{
        if(auto r1 = p1(str) ; r1) return r1;
        if(auto r2 = p2(str) ; r2) return r2;
        return {};
    };
}

/// operator >> << >>= | *

//bind operator
template<Parser P , class F>
constexpr Parser auto operator >>= (P && p , F && f){
    return bind(std::forward<P>(p) , std::forward<F>(f));
}

//plus operator
template<Parser P1 , Parser P2>
constexpr Parser auto operator |(P1 && p1 , P2 && p2)  {
    return plus(std::forward<P1>(p1) , std::forward<P2>(p2));
}

//parser a -> parser b -> parser b
template<Parser P1 , Parser P2>
constexpr Parser auto operator >> (P1 && p1 , P2 && p2 ){
    using R = typename parser_traits<P2>::type;
    return [=](parser_string str) -> parser_result<R>{
        auto result = chain_parse(str , p1 , p2);
        if(!result) return {};
        auto && [ _ , x] = result->first;
        return std::pair{std::move(x) , result->second};
    };
}

//parser a -> parser b -> parser a
template<Parser P1 , Parser P2>
constexpr Parser auto operator << (P1 && p1 , P2 && p2 ){
    using R = typename parser_traits<P1>::type;
    return [=](parser_string str)-> parser_result<R>{
        auto result = chain_parse(str , p1 , p2);
        if(!result) return {};
        auto && [ x , _ ] = result->first;
        return std::pair{std::move(x) , result->second};
    };
}

/// skip

template<Parser P>
constexpr Parser auto skip(P p){
    return p >> result(none_t{});
}

/// many , sepby , chainl , chainr

template<Parser P >
constexpr Parser auto many1(P && p){
    using T = typename parser_traits<P>::type;
    if constexpr (std::is_same_v<none_t , std::remove_cvref_t<T>>){
        return [=](parser_string str)->parser_result<none_t>{
            return foldl_parse(str , p , none_t{} , [](auto && none , auto && _){return none;});
        };
    }else{
        using list_t = cvector<T>;
        return [=](parser_string str)->parser_result<list_t>{
            return foldl_parse(str , p , list_t{} , [](list_t && c , T && v ){
                c.push_back(v);
                return std::move(c);
            });
        };
    }
}

template<Parser P >
constexpr Parser auto many(P && p){
    using T = typename parser_traits<P>::type;
    if constexpr (std::is_same_v<none_t , std::remove_cvref_t<T>>){
        return many1(p) | result(none_t{});
    }else
        return many1(p) | result_default<cvector<T>>;
}

};
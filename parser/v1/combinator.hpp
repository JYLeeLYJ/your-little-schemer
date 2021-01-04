#pragma once

#include <ranges>

#include "types.hpp"

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
constexpr auto result_def = [](parser_string str) -> parser_result<T>{
    return std::pair{T{} , str};
}; 

//parser a
template<class T>
constexpr auto zero(parser_string ) -> parser_result<T> {
    return {};
}

//parser a -> (a -> b) -> parser b
// template<Parser P, Callable F>
// requires std::invocable<F , typename parser_traits<P>::type>
// constexpr auto fmap(P && p , F && f) {
//     return [=](parser_string str){
//         auto res = p(str);
//         if(!res) return {};
//         auto b = f(res->first);
//         return parser_result<decltype(b)>{std::pair{b , res->second}};
//     };
// }

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
template<Parser T1 , Parser T2>
requires std::is_same_v<typename parser_traits<T1>::type , typename parser_traits<T2>::type>
constexpr Parser auto plus(T1 && t1 , T2 && t2){
    return [=](parser_string str)->parser_result< typename parser_traits<T1>::type >{
        if(auto r1 = t1(str); r1) return r1;
        if(auto r2 = t2(str); r2) return r2;
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
    return p1 >>= [=](auto && _){
        return p2;
    };
}

//parser a -> parser b -> parser a
template<Parser P1 , Parser P2>
constexpr Parser auto operator << (P1 && p1 , P2 && p2 ){
    return bind(p1 , [=](auto && x){
        return bind(p2 ,[=](auto && _){
            return result(std::move(x));
        });
    });
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
            auto result = p(str);
            if(!result) return {};
            while(result){
                auto && [_ , remain] = *result;
                result = p(remain);
            }
            return std::pair{none_t{},result->second};
        };
    }else{
        using list_t = cvector<T>;
        return [=](parser_string str)->parser_result<list_t>{
            list_t c{};
            auto result = p(str);
            while(result){
                auto && [v , remain] = *result;
                c.push_back(std::move(v));
                result = p(remain);
            }
            if(c.size() == 0) return {};
            else return std::pair{std::move(c),result->second};
        };
    }
}

template<Parser P >
constexpr Parser auto many(P && p){
    using T = typename parser_traits<P>::type;
    if constexpr (std::is_same_v<none_t , std::remove_cvref_t<T>>){
        return many1(p) | result(none_t{});
    }else
        return many1(p) | result_def<cvector<T>>;
}

};
#pragma once

#include <ranges>

#include "types.hpp"

//a -> parser a
template<class T>
constexpr auto result(T && t){
    return [=](parser_string str) -> parser_result<T>{
        return std::pair{t , str};
    };
}

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
template<Parser P , Callable F>
requires std::invocable<F , typename parser_traits<P>::type>
constexpr Parser auto bind(P && p , F && f){
    return [=](parser_string str){
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
template<Parser P , Callable F>
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

template<Parser P>
constexpr Parser auto skip(P p){
    return bind(p , result<none_t>);
}

/// many , sepby , chainl , chainr

// template<ListContainer Container ,Parser P >
// constexpr Parser auto many1(P && p){
//     return [=](parser_string str)->parser_result<Container>{
//         Container c{};
//         auto result = p(str);
//         while(result){
//             auto && [v , remain] = *result;
//             if constexpr (requires {c.emplace_back();})
//                 c.emplace_back(std::move(v));
//             else 
//                 c.push_back(std::move(v));
//             result = p(remain);
//         }
//         if(c.size() == 0) return {};
//         else return std::pair{std::move(c),result->second};
//     };
// }

// template<ListContainer Container , Parser P >
// constexpr Parser auto many(P && p){
//     return many1<Container>(p) | result(Container{});
// }




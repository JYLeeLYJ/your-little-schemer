#pragma once
#include <iostream>
#include "types.hpp"

namespace pscpp{

template<Parser P , Parser ...Ps>
constexpr auto chain_parse(parser_string str , P && p ,Ps && ...ps) -> parser_result<product_result_type<P , Ps...>>{
    auto result = p(str);
    if constexpr (sizeof...(ps) == 0){
        if(!result) return {};
        return std::pair{std::tuple{result->first} , result->second};
    }else{
        if(!result) return {};
        auto cons_result = chain_parse(result->second , std::forward<Ps>(ps)...);
        if(!cons_result) return {};
        return std::pair{
            std::tuple_cat(std::tuple{result->first} , cons_result->first) , 
            cons_result->second
        };
    }
}

template<Parser P , class T , class ACC>
requires std::invocable<ACC , std::add_rvalue_reference_t<std::remove_reference_t<T>> , typename parser_traits<P>::type>
constexpr auto foldl_parse(parser_string str ,P && p , T && t , ACC && acc ) -> parser_result<std::remove_cvref_t<T>>{
    auto result = p(str);
    if(!result) return {};
    for(;;){
        auto [v , remain] = std::move(*result);
        t = acc(std::move(t) , std::move(v));
        result = p(remain);
        if(!result){
            return std::pair{std::move(t) , remain};
        }
    };
}

};

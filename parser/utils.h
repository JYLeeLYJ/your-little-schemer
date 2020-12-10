#pragma once

#include<tuple>
#include<type_traits>
template<class T>
struct is_tuple: std::false_type{};

template<class ...Args>
struct is_tuple<std::tuple<Args...>> : std::true_type{};

template<class T1  , class T2>
constexpr auto cat_tuple(T1 && t1 , T2 && t2){
    if constexpr (is_tuple<T1>{})
        if constexpr (is_tuple<T2>{})
            return std::tuple_cat(std::forward<T1>(t1) , std::forward<T2>(t2));
        else 
            return std::tuple_cat(std::forward<T1>(t1) , std::tuple{std::forward<T2>(t2)});
    else 
        if constexpr (is_tuple<T2>{})
            return std::tuple_cat(std::tuple{std::forward<T1>(t1)} , std::forward<T2>(t2));
        else 
            return std::tuple(std::forward<T1>(t1) , std::forward<T2>(t2));
}
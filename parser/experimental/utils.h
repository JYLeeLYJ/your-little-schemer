#pragma once
#include <numeric>
#include <forward_list>
#include <ranges>

template<std::integral T>
constexpr T char_list_to_integer(const std::forward_list<char> & list){
    auto v = list | std::ranges::views::transform([](char ch)->T {return ch - '0';}) ;
    return std::accumulate(v.begin() , v.end() , T{} , [](T init , T i ){return init * 10 + i;});
}
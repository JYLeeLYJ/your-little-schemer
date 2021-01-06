#pragma once
#include <type_traits>
#include <experimental/type_traits>

template<template <typename ...> class kind>
struct hkt{
    template<class T>
    struct match_kind : std::false_type{};

    template<class ...Arg>
    struct match_kind<kind<Arg...>> : std::true_type{}; 
};

template<class T, typename... Args>
using can_invoke_t = decltype(std::declval<T>()(std::declval<Args>()...));

template<typename T, typename... Args>
constexpr bool can_invoke = std::experimental::is_detected_v<can_invoke_t, T, Args...>;

template <typename F, typename...Arguments>
struct curry_t {
    template <typename...Args>
    constexpr decltype(auto) operator()(Args&&...a)  const {
        curry_t<F, Arguments..., Args...> cur = { f_,
            std::tuple_cat(args_, std::make_tuple(std::forward<Args>(a)...)) };

        if constexpr (!can_invoke<F, Arguments..., Args...>) 
            return cur;
        else 
            return cur();
    }

    constexpr decltype(auto) operator () () const {
        return std::apply(f_, args_);
    }

    F f_;
    std::tuple<Arguments...> args_;
};

template<typename F>
constexpr curry_t<F> make_curry(F&& f) {
    return { std::forward<F>(f) };
}

template<class T>
struct constructor{
    explicit constexpr constructor() noexcept = default;
    template<class ...Ts>
    requires std::is_constructible_v<T , Ts...>
    constexpr auto operator() (Ts && ...ts){ return T(std::forward<Ts>(ts)...);}
};
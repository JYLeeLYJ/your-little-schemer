#pragma once

#include <string_view>
#include <functional>
#include <optional>
#include <type_traits>
#include <variant>
#include <experimental/type_traits>

using parser_string = std::string_view;

template<class T>
using parser_result = std::optional<std::pair<T , parser_string>>;

template<class T>
using result_type = std::invoke_result_t<T ,std::string_view>;

template<class T>
struct is_parser_result 
: std::false_type{};

template<class T>
struct is_parser_result<parser_result<T>>
: std::true_type{};

template<class T >
concept parseable = is_parser_result<result_type<T>>{} == true;

template<parseable T>
struct parser_traits:std::true_type{
    using type = typename result_type<T>::value_type::first_type;
};

template<class F>
concept callable = requires (F f){
    typename decltype(std::function{f})::result_type;
};

template<class F , class P>
concept bindable = requires(F f , P p){
    parseable<P>;
    parseable<std::invoke_result_t<F ,typename parser_traits<P>::type>>;
};

template<class T1 , class T2>
concept same_parser = 
    parseable<T1> && 
    parseable<T2> && 
    std::is_same_v<typename parser_traits<T1>::type ,typename parser_traits<T2>::type>;

//concrete function type erase
template<class T>
using parser_t = std::function<parser_result<T>(parser_string)>;

template<class T>
constexpr auto parser(parser_result<T>(* p)(parser_string)){
    return [=](parser_string str)constexpr{return p(str);};
}

// simple type system

// kind should be class template 
// alias template will lead to mismatch problem on gcc10 or clang10 
template<template <typename ...> class kind>
struct hkt{
    template<class T>
    struct match_kind : std::false_type{};

    template<class ...Arg>
    struct match_kind<kind<Arg...>> : std::true_type{}; 
};

//denote product type
template<class ...T>
struct product_type {

    using tuple_type = std::tuple<T...>;

    tuple_type tp;

    constexpr product_type() = default;
    constexpr product_type(std::tuple<T...> _tp)
    :tp(std::move(_tp)){}
};

template<class T>
constexpr bool is_product = hkt<product_type>::match_kind<T>{};

template<class L , class R>
constexpr auto product(L && lhs, R && rhs){
    using TL = std::remove_cvref_t<L>;
    using TR = std::remove_cvref_t<R>;
    if constexpr (is_product<TL>)
        if constexpr(is_product<TR>) 
            return product_type{std::tuple_cat(std::forward<L>(lhs).tp , std::forward<R>(rhs).tp)};
        else 
            return product_type{std::tuple_cat(std::forward<L>(lhs).tp , std::tuple<TR>{std::forward<R>(rhs)})};
    else 
        if constexpr (is_product<TR>)
            return product_type{std::tuple_cat(std::tuple<TL>{std::forward<L>(lhs)} , std::forward<R>(rhs).tp)};
        else 
            return product_type{std::tuple(std::forward<L>(lhs) , std::forward<R>(rhs))};
}

template<class ...Types>
class std::tuple_size<product_type<Types...>>
:public std::integral_constant<std::size_t, sizeof...(Types)>{};

template<std::size_t I ,class ...Types>
struct std::tuple_element<I , product_type<Types...>>{
    using type = typename std::tuple_element<I,typename product_type<Types...>::tuple_type>::type;
};

template<std::size_t I , class ...Types>
constexpr decltype(auto) get(product_type<Types...> & t) noexcept{
    return std::get<I>(t.tp);
}

template<std::size_t I , class ...Types>
constexpr decltype(auto) get(product_type<Types...> && t) noexcept{
    return std::get<I>(std::move(t).tp);
}

template<std::size_t I , class ...Types>
constexpr decltype(auto) get(const product_type<Types...> & t) noexcept{
    return std::get<I>(t.tp);
}

template<std::size_t I , class ...Types>
constexpr decltype(auto) get(const product_type<Types...> && t) noexcept{
    return std::get<I>(std::move(t).tp);
}

constexpr inline decltype(auto) fix(auto&& f){
    auto g = [&](auto&& h){
        return [&](auto&& ...x) { 
            return f(h(h))(std::forward<decltype(x)>(x)...);
        };
    };
    return g(g);
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
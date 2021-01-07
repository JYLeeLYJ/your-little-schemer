#pragma once

#include <type_traits>
#include <optional>
#include <string_view>
#include <functional>
#include <concepts>

#include "functional.hpp"

namespace pscpp{

template<class T>
class parser_result: public std::optional<std::pair<T , std::string_view>>{
public:
    using std::optional<std::pair<T , std::string_view>>::optional;
    using type = T;

    constexpr parser_result(T t , std::string_view s) noexcept
    : std::optional<std::pair<T , std::string_view>>{std::pair{std::move(t) , s}}{}
};

using parser_string = std::string_view;

template<class T>
using parser_func_t = parser_result<T> (*)(parser_string);

/// type erase for non capture constexpr lambda 
template<class T>
class parser_lambda_t{
public:
    constexpr parser_lambda_t(const parser_func_t<T> f) noexcept  :_f(f){}
    constexpr parser_result<T> operator() (parser_string str) const noexcept{return _f(str);}
private:
    parser_func_t<T> _f;
};

template<class T>
constexpr bool is_parser_result = hkt<parser_result>::match_kind<T>{};

template<class T>
concept Parser = is_parser_result<std::invoke_result_t<T , parser_string>>;

template<Parser P>
struct parser_traits {
    using type = typename std::invoke_result_t<P,parser_string>::type;
};

template<class T>
concept Callable = requires(T t){
    typename decltype(std::function{t})::result_type;
};

template<class F , class ...Args>
concept FMapFn = (... && Parser<Args>) &&
    !std::is_same_v<void , std::invoke_result_t<F , typename parser_traits<Args>::type...>>;


template<Parser ...Ps>
using product_result_type = std::tuple<typename parser_traits<Ps>::type ...>;

struct none_t {
    constexpr bool operator==(const none_t & rhs) const {return true;}
};

};


#include <type_traits>
#include <optional>
#include <string_view>
#include <functional>

#include "functional.hpp"

template<class T>
class parser_result: std::optional<std::pair<T , std::string_view>>{
public:
    using std::optional<std::pair<T , std::string_view>>::optional;
    using std::optional<std::pair<T , std::string_view>>::operator ==();

    using type = T;
};

using parser_string = std::string_view;

template<class T>
using parser_func_t = parser_result<T> (*)(parser_string);

template<class T>
class parser_t{
public:
    constexpr parser_t(parser_func_t<T> f) noexcept  :_f(f){}
    parser_result operator() (parser_string str)  noexcept{return _f(str);}
private:
    parser_func_t _f;
};

template<class T>
constexpr bool is_parser_result = hkt<parser_result>::match_kind<T>{};

template<class T>
concept Parser = is_parser_result<std::invoke_result_t<T , parser_string>>;

template<class T>
concept Callable = requires(T t){
    typename decltype(std::function{t})::result_type;
};

template<Parser P>
struct parser_traits {
    using type = typename std::invoke_result_t<P,parser_string>::type;
};
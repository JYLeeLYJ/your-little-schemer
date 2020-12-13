#pragma once
#include "types.h"

//basic monadic parser

//parser a
template<class T>
auto zero (parser_string str) -> parser_result<T>{
    return {} ;
}

//parser char
auto item (parser_string str) -> parser_result<char>{
    return std::pair{str[0] , str.substr(1)};
}

//basic monadic combinator

//a -> parser a
template<class T>
parseable auto result(T && t) -> auto{
    return [t = std::forward<T>(t)]
    (parser_string str)->parser_result<std::remove_reference_t<T>>{
        return std::pair{t , str};
    };
}

//parser a -> (a->parser b) -> parser b
template<parseable P , bindable<P> F>
parseable auto bind(P && p , F && f) {
    using T1 = parser_traits<P>::type;                             //a
    using T2 = parser_traits<std::invoke_result_t<F , T1>>::type;   //b
    auto ret_p = [p = std::forward<P>(p) , f = std::forward<F>(f)]
    (parser_string str)->parser_result<T2>{
        auto r1 = p(str);
        if(!r1) return {};
        else return f(std::move(r1->first))(r1->second);
    };
    return parser_t<T2>(std::move(ret_p));
}

//parser a -> parser b -> parser (a , b)
template<parseable P1 , parseable P2>
parseable auto seq(P1 && p1 , P2 && p2) {
    using T1 = typename parser_traits<P1>::type;
    using T2 = typename parser_traits<P2>::type;
    using Ret = decltype(product(std::declval<T1>() , std::declval<T2>()));

    auto ret_p = [p1 = std::forward<P1>(p1) , p2 = std::forward<P2>(p2)](parser_string str)->parser_result<Ret>{
        if      (auto r1 = p1(str);!r1) return std::nullopt;
        else if (auto r2 = p2(r1->second);!r2) return std::nullopt;
        else    return  std::pair{product(r1->first , r2->first) , r2->second};
    };
    return parser_t<Ret>(std::move(ret_p));
}

//parser a -> parser a -> parser a
template<parseable P1 , parseable P2>
requires same_parser<P1 ,P2>
parseable auto plus(P1 && p1 , P2 && p2)  {
    using T1 = typename parser_traits<P1>::type;
    auto ret_p = [p1 = std::forward<P1>(p1) , p2 = std::forward<P2>(p2)](parser_string str)->parser_result<T1>{
        if(auto r1 = p1(str); r1) return r1;
        if(auto r2 = p2(str); r2) return r2;
        return {};
    };
    return parser_t<T1>(std::move(ret_p));
};

//suger operator : >>= + | >> <<

template<parseable P1 , bindable<P1> F>
parseable auto operator >>= (P1 && p1 , F && f){
    return bind(std::forward<P1>(p1) , std::forward<F>(f));
}

template<parseable P1 , parseable P2>
parseable auto operator + (P1 && p1 , P2 && p2){
    return seq(std::forward<P1>(p1) , std::forward<P2>(p2));
}

template<parseable P1 , parseable P2>
requires same_parser<P1 ,P2>
parseable auto operator |(P1 && p1 , P2 && p2)  {
    return plus(std::forward<P1>(p1) , std::forward<P2>(p2));
}

// more combinators

// (char -> bool) -> parser char
template<std::predicate<char> Pred>
parseable auto satisfy(Pred && predicate) {
    return item 
    >>= [p = std::forward<Pred>(predicate)](char x)-> parser_t<char>{
        if (p(x)) return result(x);  
        else return zero<char>;
    };
}

// a -> a -> parser a
template<class T>
parseable auto range(T l , T h){
    return satisfy([=](const T & t){ return t >= l && t <= h;});
}

//high-level combinators

//char -> parser char
parseable auto onechar(char ch){
    return satisfy([=](char x)->bool{return x == ch;});
}

//string -> parser string
// auto string(std::string_view word) -> parser_t<std::string_view>{
//     using namespace std::literals;
//     if(word.empty()) return result(""sv);
//     else return 
//         onechar(word[0])        >>= [=](char _)             ->  auto{return 
//         string(word.substr(1))  >>= [=](std::string_view _) ->  auto{return
//             result(word);
//         };};
// };
auto string(std::string_view word) -> parser_t<std::string_view>{
    using namespace std::literals;
    if(word.empty()) return result(""sv);
    else return 
        (onechar(word[0]) + string(word.substr(1)))
        >>= [=](auto && _) {return 
            result(word);
        };
};

//many1 , many

//sepby1 , sepby
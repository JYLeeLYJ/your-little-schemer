#pragma once

#include <forward_list>

#include "types.h"

//basic monadic definition 

//monad , monad plus , applicative , functor

//parser a
template<class T>
auto zero (parser_string str) -> parser_result<T>{
    return {} ;
}

//parser char
auto item (parser_string str) -> parser_result<char>{
    if(str.empty()) return {};
    else            return std::pair{str[0] , str.substr(1)};
}

//basic monadic combinator

//a -> parser a
template<class T>
parseable auto result(T && t) {
    return [t = std::forward<T>(t)]
    (parser_string str)->parser_result<std::remove_reference_t<T>>{
        return std::pair{t , str};
    };
}

//parser a -> (a->parser b) -> parser b
template<parseable P , bindable<P> F>
parseable auto bind(P && p , F && f) {
    using T1 = typename parser_traits<P>::type;                             //a
    using T2 = typename parser_traits<std::invoke_result_t<F , T1>>::type;   //b
    auto ret_p = [=]
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

    auto ret_p = [=](parser_string str)->parser_result<Ret>{
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
    auto ret_p = [=](parser_string str)->parser_result<T1>{
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
    >>= [=](char x)-> parser_t<char>{
        if (predicate(x)) return result(x);  
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
auto string(std::string_view word) -> parser_t<std::string_view>{
    using namespace std::literals;
    if(word.empty()) return result(""sv);
    else return 
        (onechar(word[0]) + string(word.substr(1))) 
        >>= [=](auto && _) {return result(word);};
};

//Repetition

//many1 , many
template<parseable P>
auto many(P && p) -> parser_t<std::forward_list<typename parser_traits<P>::type>>{
    using data_type = typename parser_traits<P>::type;
    using list_type = std::forward_list<data_type>;

    return plus(
        p       >>= [=](data_type x)                     -> auto {return
        many(p) >>= [x = std::move(x)](list_type && xs)  -> auto {return 
            result((xs.push_front(std::move(x)) , std::move(xs)));
        };},
        result(list_type{})
    );
} 

template<parseable P>
parseable auto many1(P && p){
    return (p + many(p)) >>= [](auto && tp){
        auto && [x ,xs] = tp.tp;
        return result((xs.push_front(std::move(x)) , std::move(xs)));
    };
}

//sepby1 , sepby
//parser a -> parser b -> parser [a]
template<parseable P , parseable Sep>
parseable auto sepby1( P && p , Sep && sep) {
    auto repeat = (sep + p) >>= [](auto && tp){
        return result(std::move(std::get<1>(tp.tp)));
    };
    return (p + many(repeat)) 
    >>= [](auto && tp){
        auto && [x , xs] = tp.tp;
        return result((xs.push_front(std::move(x)) , std::move(xs)));
    };
}

template<parseable P , parseable Sep>
parseable auto sepby(P && p , Sep && sep){
    using list_type = typename parser_traits<P>::type;
    return sepby1(std::forward<P>(p) , std::forward<Sep>(sep)) | result(list_type{});
}

template<parseable Open , parseable P , parseable Close>
parseable auto bracket( Open && open , P && p ,  Close && close){
    return (open + p + close) 
    >>= [](auto && tp){
        auto && [_ , ls , __] = tp.tp;
        return result(std::move(ls));
    };
}

//parser a -> parser (a -> a -> a) -> parser a
template<parseable T , parseable Op>
auto chainl1(T && p , Op && op) -> parser_t<typename parser_traits<T>::type> {
    using TA = typename parser_traits<T>::type ;
    using TOp= typename parser_traits<Op>::type;

    static_assert(std::is_invocable_v<TOp , TA , TA>);

    return (p + many(op + p))
        >>= [](auto && tp){
            auto && [ x , fys ] = tp.tp;
            for(auto && fy : fys){
                auto && [op , y] = fy.tp;
                x = op(x , y);
            }
            return result(x);
        };
}

//chainr1
template<parseable T , parseable Op>
auto chainr1(T && p , Op && op) -> parser_t<typename parser_traits<T>::type >{
    using TA = typename parser_traits<T>::type ;
    using TOp= typename parser_traits<Op>::type;

    static_assert(std::is_invocable_v<TOp , TA , TA>);

    return p >>= [=](auto && x){ return(
        op              >>= [=](auto && f) {return 
        chainr1(p , op) >>= [=](auto && y) {return
            f(x , y);
        }; })
        | result(x);
    };
}

//chainl

template<parseable T1 , parseable Op , parseable T2>
requires same_parser<T1,T2>
auto chainl(T1 && p , Op && op , T2 &&  v) -> parser_t<typename parser_traits<T1>::type >{
    return chainl1(std::forward<P>(p) , std::forward<Op>(op)) | result(std::forward<T2>(v));
}
//chainr

template<parseable T1 , parseable Op , parseable T2>
requires same_parser<T1,T2>
auto chainr(T1 && p , Op && op , T2 &&  v) -> parser_t<typename parser_traits<T1>::type >{
    return chainr1(std::forward<P>(p) , std::forward<Op>(op)) | result(std::forward<T2>(v));
}
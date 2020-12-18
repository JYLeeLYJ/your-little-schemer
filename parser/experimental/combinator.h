#pragma once

#include <forward_list>

#include "types.h"

//basic monadic definition 

//monad
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

//monad plus
//parser a
template<class T>
auto zero (parser_string str) -> parser_result<T>{
    return {} ;
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

//applicative 
//lift : f -> parser f
template<callable F>
parseable auto lift(F && f) {
    return result(make_curry(std::forward<F>(f)));
}

template<parseable F , parseable P>
requires std::is_invocable_v<typename parser_traits<F>::type , typename parser_traits<P>::type>
parseable auto apply(F && pf , P && p){
    return bind( pf , [=](auto && f){
        return bind(p , [f](auto && x){
            return result(f(std::move(x)));
        });
    });
}

//functor
template<callable F , parseable P>
parseable auto fmap(F && f , P && p){
    return bind(p , [f = make_curry(f)](auto && x){
        return result( f(std::move(x)) );
    });
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

//suger operator : >>= + | >> <<
//note : >>= is right associative , we need to implement F1 'bind' F2 to satisfy associative law
template<parseable P1 , bindable<P1> F>
parseable auto operator >>= (P1 && p1 , F && f){
    return bind(std::forward<P1>(p1) , std::forward<F>(f));
}

template<parseable P1 , parseable P2>
parseable auto operator + (P1 && p1 , P2 && p2){
    return seq(std::forward<P1>(p1) , std::forward<P2>(p2));
    // return apply(std::forward<P1>(p1) ,std::forward<P2>(p2));
}

template<parseable F , parseable P>
parseable auto operator * (F && f , P && p ){
    return apply(std::forward<F>(f) , std::forward<P>(p));
}

template<parseable P1 , parseable P2>
requires same_parser<P1 ,P2>
parseable auto operator |(P1 && p1 , P2 && p2)  {
    return plus(std::forward<P1>(p1) , std::forward<P2>(p2));
}

template<parseable P1 , parseable P2>
parseable auto operator >> (P1 && p1 , P2 && p2) {
    return seq(std::forward<P1>(p1) , std::forward<P2>(p2)) 
        >>= [](auto && tp){
            auto && [_ , r] = tp;
            return result(std::move(r));
        };
}

template<parseable P1 , parseable P2>
parseable auto operator << (P1 && p1 , P2 && p2){
    return seq(std::forward<P1>(p1) , std::forward<P2>(p2)) 
            >>= [](auto && tp){
                auto && [l , _] = tp;
                return result(std::move(l));
            };
}

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
        auto && [x ,xs] = tp;
        return result((xs.push_front(std::move(x)) , std::move(xs)));
    };
}

//sepby1 , sepby
//parser a -> parser b -> parser [a]
template<parseable P , parseable Sep>
parseable auto sepby1( P && p , Sep && sep) {
    return (p + many(sep >> p)) 
    >>= [](auto && tp){
        auto && [x , xs] = tp;
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
    return (open >> p << close) ;
}

//parser a -> parser (a -> a -> a) -> parser a
template<parseable T , parseable Op>
auto chainl1(T && p , Op && op) -> parser_t<typename parser_traits<T>::type> {
    using TA = typename parser_traits<T>::type ;
    using TOp= typename parser_traits<Op>::type;

    static_assert(std::is_invocable_v<TOp , TA , TA>);

    return (p + many(op + p))
        >>= [](auto && tp){
            auto && [ x , fys ] = tp;
            for(auto && fy : fys){
                auto && [op , y] = fy;
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
            result (f(x , y));
        }; })
        | result(x);
    };
}

//chainl

template<parseable T1 , parseable Op , parseable T2>
requires same_parser<T1,T2>
auto chainl(T1 && p , Op && op , T2 &&  v) -> parser_t<typename parser_traits<T1>::type >{
    return chainl1(std::forward<T1>(p) , std::forward<Op>(op)) | result(std::forward<T2>(v));
}
//chainr

template<parseable T1 , parseable Op , parseable T2>
requires same_parser<T1,T2>
auto chainr(T1 && p , Op && op , T2 &&  v) -> parser_t<typename parser_traits<T1>::type >{
    return chainr1(std::forward<T1>(p) , std::forward<Op>(op)) | result(std::forward<T2>(v));
}
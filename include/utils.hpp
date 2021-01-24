#pragma once

#include <variant>

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

inline constexpr auto default_visitor = [](auto && _){};

template<class ...Ts>
class variant_base : public std::variant<Ts...>{
public :
    using std::variant<Ts...>::variant;
    constexpr ~variant_base() = default;

    template<class V>
    constexpr void match(V && vis) const { 
        std::visit( std::forward<V>(vis) , var());
    }
    template<class V>
    constexpr void match(V && vis){
        std::visit(std::forward<V>(vis), var());
    }
    template<class R , class V>
    constexpr R match(V && vis)  {
        return std::visit<R>(std::forward<V>(vis) , var());
    }
    template<class R , class V>
    constexpr R match(V && vis) const {
        return std::visit<R>(std::forward<V>(vis) , var());
    }

    template<class T>
    constexpr bool holds() const {
        return std::holds_alternative<T>(*this);
    }

    template<class T>
    constexpr decltype(auto) get() { return std::get<T>(var());}
    template<class T>
    constexpr decltype(auto) get_if() {return std::get_if<T>(&var());}
    template<class T>
    constexpr decltype(auto) get() const{ return std::get<T>(var());}
    template<class T>
    constexpr decltype(auto) get_if() const {return std::get_if<T>(&var());}

protected:
    constexpr std::variant<Ts...> & var(){return *this;}
    constexpr const std::variant<Ts...> & var() const {return *this;}
};
#pragma once

#include <type_traits>
#include <optional>
#include <string_view>
#include <functional>
#include <version>
#include <concepts>

#include "functional.hpp"

namespace pscpp{

template<class T>
class parser_result: public std::optional<std::pair<T , std::string_view>>{
public:
    using std::optional<std::pair<T , std::string_view>>::optional;
    using type = T;
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

template<class T>
concept Callable = requires(T t){
    typename decltype(std::function{t})::result_type;
};

template<Parser P>
struct parser_traits {
    using type = typename std::invoke_result_t<P,parser_string>::type;
};

template<Parser ...Ps>
using product_result_type = std::tuple<typename parser_traits<Ps>::type ...>;


struct none_t {};

constexpr bool operator==(const none_t & lhs , const none_t & rhs){
    return true;
}

#ifndef __cpp_lib_constexpr_vector
#include <memory>
#include <new>
#include <initializer_list>
///simple vector support constexpr , cause constexpr std::vector is still unsupported.
template<class T>
class cvector : protected std::allocator<T>{
public:
    constexpr cvector() {
        preserve(4);
    }
    constexpr cvector(const std::initializer_list<T> & ls) {
        preserve(1.5 * ls.size());
        for (std::size_t i = 0 ; const auto & x : ls){
            std::construct_at(_start+i , x); 
            ++i;
        }
        _n = ls.size();
    }

    constexpr cvector(const cvector & v){
        preserve(v.capacity());
        for (std::size_t i = 0 ; const auto & x : v){
            std::construct_at(_start+i , x); 
            ++i;
        }
        _n = v.size();
    }

    constexpr cvector(cvector && v) noexcept{
        move_values_from(v);
    }

    constexpr ~cvector() noexcept{
        if(!_start) return ;
        deconstruct_all();
        this->deallocate(_start ,capacity());
        _n_storage = _n = 0;
        _start = nullptr;
    }

    constexpr cvector& operator = (const cvector & v){
        deconstruct_all();
        preserve(v.capacity());
        for(std::size_t i = 0 ; auto & x : v){
            std::construct_at(_start+i , x); 
            ++i;
        }
        _n = v.size();
        return *this;
    }

    constexpr cvector & operator = (cvector && v) noexcept{
        this->~cvector();
        move_values_from(v);
        return *this;
    }

public:
    using iterator = T *;
    using const_iterator = const T *;
public:
    constexpr std::size_t size() const { return _n;}
    constexpr std::size_t capacity() const {return _n_storage;}
    constexpr bool empty() const {return _n == 0;}

    constexpr iterator begin(){return _start;}
    constexpr iterator end(){return _start+_n;}
    constexpr const_iterator begin() const {return _start;}
    constexpr const_iterator end() const {return _start + _n;}

    constexpr void push_back(T t){
        if(size() == capacity()) 
            preserve(2 * size());
        std::construct_at(_start + size() , std::move(t));
        ++_n;
    }

    constexpr T & operator[] (std::size_t i){
        return _start[i];
    }

    constexpr const T & operator [](std::size_t i) const{
        return _start[i];
    }

    constexpr bool operator== (const cvector & v) const{
        if(size() != v.size()) return false;
        for(std::size_t i = 0 ; auto & x : v){
            if(_start[i] != x) return false;
            ++i;
        }
        return true;
    }

private:

    constexpr void preserve(std::size_t n){
        if(n == 0) n = 4;
        if(_start == nullptr) {
            _start = this->allocate(n);
            if(_start == nullptr) throw std::bad_alloc{};
            _n_storage = n;
        }
        else if( capacity() < n){
            T* p = this->allocate(n);
            if(p == nullptr)  throw std::bad_alloc{};
            for(std::size_t i = 0 ; i < size() ; ++ i)
                p[i] = std::move(_start[i]);
            this->deallocate(_start , _n_storage);
            _n_storage = n;
            _start = p;
        }
    }

    constexpr void deconstruct_all(){
        std::destroy_n(begin() , size());
        _n = 0;
    }

    constexpr void move_values_from(cvector & v){
        _n = v.size();              v._n = 0;
        _n_storage = v.capacity();  v._n_storage = 0;
        _start = v._start;          v._start = nullptr;
    }

private:
    std::size_t _n{0};
    std::size_t _n_storage{0};
    T * _start{nullptr};
};
#endif

};


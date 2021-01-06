#pragma once

#include <type_traits>
#include <memory>
#include <new>
#include <version>

namespace cexpr{

template<class T>
requires (!std::is_reference_v<T>)
class Box : protected std::allocator<T>{
public:
    using element_type = T;
public:
    explicit constexpr Box(T x) : _ptr(this->allocate(1)){
        std::construct_at(_ptr , std::move(x));
    }

    template<class ...Ts>
    requires std::constructible_from<T , Ts...>
    explicit constexpr Box(Ts ... ts) : _ptr(this->allocate(1)){
        std::construct_at(_ptr , std::forward<Ts>(ts)...);
    }

    constexpr Box(const Box & box) = delete;

    constexpr Box& operator= (const Box & box) = delete;

    constexpr Box(Box && box) noexcept {
        destroy_value();
        std::swap(_ptr , box._ptr);
    }

    constexpr Box & operator= (Box && box) noexcept{
        destroy_value();
        std::swap(_ptr , box._ptr);
    }

    constexpr ~Box() noexcept{
        destroy_value();
    }

public:
    constexpr T * get() const noexcept { return _ptr;}
    constexpr T & operator * () const { return *_ptr;}
    constexpr T * operator ->() const noexcept { return _ptr;}
    explicit constexpr operator bool () const noexcept {return _ptr != nullptr;}
private:
    constexpr void destroy_value(){
        if(_ptr){
            std::destroy_at(_ptr);
            this->deallocate(_ptr,1);
        }
        _ptr = nullptr;
    }
private:
    T * _ptr{nullptr};
};

#ifndef __cpp_lib_constexpr_vector
///simple vector support constexpr , cause constexpr std::vector is still unsupported.
template<class T>
class vector : private std::allocator<T>{
public:
    constexpr vector() {
        preserve(4);
    }
    constexpr vector(const std::initializer_list<T> & ls) {
        preserve(1.5 * ls.size());
        for (std::size_t i = 0 ; const auto & x : ls){
            std::construct_at(_start+i , x); 
            ++i;
        }
        _n = ls.size();
    }

    constexpr vector(const vector & v){
        preserve(v.capacity());
        for (std::size_t i = 0 ; const auto & x : v){
            std::construct_at(_start+i , x); 
            ++i;
        }
        _n = v.size();
    }

    constexpr vector(vector && v) noexcept{
        move_values_from(v);
    }

    constexpr ~vector() noexcept{
        if(!_start) return ;
        deconstruct_all();
        this->deallocate(_start ,capacity());
        _n_storage = _n = 0;
        _start = nullptr;
    }

    constexpr vector& operator = (const vector & v){
        deconstruct_all();
        preserve(v.capacity());
        for(std::size_t i = 0 ; auto & x : v){
            std::construct_at(_start+i , x); 
            ++i;
        }
        _n = v.size();
        return *this;
    }

    constexpr vector & operator = (vector && v) noexcept{
        this->~vector();
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

    constexpr bool operator== (const vector & v) const{
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

    constexpr void move_values_from(vector & v){
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

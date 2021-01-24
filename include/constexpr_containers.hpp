#pragma once

#include <type_traits>
#include <memory>
#include <new>
#include <version>
#include <atomic>

namespace cexpr{

template<class T>
requires (!std::is_reference_v<T> && !std::is_void_v<T>)
class box : protected std::allocator<T>{
public:
    using element_type = T;
public:
    explicit constexpr box(T x) : _ptr(this->allocate(1)){
        std::construct_at(_ptr , std::move(x));
    }

    template<class ...Ts>
    requires std::constructible_from<T , Ts...>
    explicit constexpr box(Ts ... ts) : _ptr(this->allocate(1)){
        std::construct_at(_ptr , std::forward<Ts>(ts)...);
    }

    constexpr box(const box & b) = delete;

    constexpr box& operator= (const box & b) = delete;

    constexpr box(box && b) noexcept {
        destroy_value();
        std::swap(_ptr , b._ptr);
    }

    constexpr box & operator= (box && b) noexcept{
        destroy_value();
        std::swap(_ptr , b._ptr);
    }

    constexpr ~box() noexcept{
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

struct ref_count_safe_t{
    mutable std::atomic<uint32_t> ref_cnt{1};
    auto add_ref() const {
        return ref_cnt.fetch_add(1 ,std::memory_order_relaxed);
    }
    auto sub_ref() const {
        return ref_cnt.fetch_sub(1 , std::memory_order_acq_rel);
    }
    auto cnt() const {
        return ref_cnt.load(std::memory_order_relaxed);
    }
};

struct ref_count_t{
    mutable uint32_t ref_cnt{1};
    constexpr auto cnt()    const {return ref_cnt;}
    constexpr auto add_ref() const { return ++ref_cnt;}
    constexpr auto sub_ref() const { return --ref_cnt;}
    auto &safe() const { return reinterpret_cast<const ref_count_safe_t&>(*this);}
};

template<bool thread_safe , class T>
struct ref_count_storage {
    using ref_count = std::conditional_t<thread_safe , ref_count_safe_t , ref_count_t>;
    T data;
    ref_count count{};

    constexpr ref_count_storage(T &&dt) noexcept
    : data(std::move(dt)) {}

    constexpr ref_count_storage(const T & dt) 
    : data(dt) {}
};

//default immutable and copy on write , 
//thread safe atomic ref cnt at runtime
template<class T>
requires (!std::is_reference_v<T> && !std::is_void_v<T>)
class cow : protected std::allocator<ref_count_storage<false , T>>{
    using block_t = ref_count_storage<false , T>;
public:
    using value_type = T;
public:
    constexpr cow() :_ptr(this->allocate(1)){
        static_assert(std::is_default_constructible_v<T>);
        std::construct_at(_ptr, T{});
    }

    constexpr cow(T t) : _ptr(this->allocate(1)){
        std::construct_at(_ptr , std::move(t));
    }
    constexpr ~cow() noexcept {
        if(!_ptr || sub_ref() != 0) return ;
        std::destroy_at(_ptr);
        this->deallocate(_ptr , 1);
        _ptr = nullptr;
    }
    constexpr cow(const cow & c) noexcept
    : _ptr(c._ptr){
        if(c._ptr) c.add_ref();
    }
    constexpr cow & operator = (const cow & c)noexcept{
        if(_ptr == c._ptr) return *this;
        this->~cow();
        if(c._ptr) c.add_ref();
        _ptr = c._ptr;
        return *this;
    }
    constexpr cow(cow && c) noexcept{
        _ptr = c._ptr;
        c._ptr = nullptr;
    }
    constexpr cow & operator= (cow && c) noexcept {
        this->~cow();
        _ptr = c._ptr ; c._ptr = nullptr;
        return *this;
    }

    constexpr const T & operator * () const noexcept { return _ptr->data;}
    constexpr const T * operator ->() const noexcept { return &(_ptr->data);}

    constexpr bool operator == (const cow & rhs) const noexcept
    requires(std::equality_comparable<T>){
        return (!_ptr && !rhs._ptr) || ref() == rhs.ref();
    }
    constexpr bool operator == (const T & rhs) const noexcept
    requires(std::equality_comparable<T>) {
        return _ptr && (ref() == rhs);
    }

public:
    constexpr T & into_owned() {
        if(cnt() == 1) return _ptr->data;
        block_t * new_ptr = this->allocate(1);
        std::construct_at(new_ptr , _ptr->data);
        this->~cow();
        _ptr = new_ptr;
        return _ptr->data;
    }
    constexpr cow clone() const{
        return cow{_ptr->data};
    }
    constexpr const T & ref() const {
        return _ptr->data;
    }
    constexpr T & mut() {
        return into_owned();
    }
    constexpr auto cnt() const {
        return std::is_constant_evaluated() ? _ptr->count.cnt() : _ptr->count.safe().cnt();
    }
private:
    constexpr auto sub_ref() const {
        if(std::is_constant_evaluated()) return _ptr->count.sub_ref();
        else return _ptr->count.safe().sub_ref();
    }
    constexpr auto add_ref() const {
        if(std::is_constant_evaluated()) return _ptr->count.add_ref();
        else return _ptr->count.safe().add_ref();
    }

protected:
    block_t * _ptr{nullptr};
};

//simple vector support constexpr , 
//cause constexpr std::vector is still unsupported.
template<class T>
class vector : private std::allocator<T>{
public:
    constexpr vector()  = default;
    constexpr vector(const std::initializer_list<T> & ls) {
        preserve(1.5 * ls.size());
        uninitialize_copy_all(_start, ls);
        _n = ls.size();
    }

    constexpr vector(const vector & v){
        preserve(v.capacity());
        uninitialize_copy_all(_start ,v);
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
        if(std::addressof(v) == this) return *this;
        if(_start )deconstruct_all();
        preserve(v.capacity());
        uninitialize_copy_all(_start , v);
        _n = v.size();
        return *this;
    }

    constexpr vector & operator = (vector && v) noexcept{
        if(std::addressof(v) == this) return *this;
        if(_start)deconstruct_all();
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
        if(size() == capacity()) preserve( size() ? 2 * size() : 4 ) ;
        std::construct_at(_start + size() , std::move(t));
        ++_n;
    }

    template<class ...TArgs>
    requires std::constructible_from<T , TArgs...>
    constexpr void emplace_back(TArgs && ...args) {
        if(size() == capacity()) preserve( size() ? 2 * size() : 4 ) ;
        std::construct_at(_start+size() , std::forward<TArgs>(args)...);
        ++ _n ;
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
        if(n == 0 ) return ;   // do nothing
        if(_start == nullptr) {
            _start = this->allocate(n);
            if(_start == nullptr) throw std::bad_alloc{};
            _n_storage = n;
        }
        else if( capacity() < n){
            T* p = this->allocate(n);
            if(p == nullptr)  throw std::bad_alloc{};
            uninitialize_move_all(p , *this);
            this->deallocate(_start , _n_storage);
            _n_storage = n;
            _start = p;
        }
    }
    
    //TODO :optimize for pod
    template<std::ranges::range V>
    requires std::same_as<T , typename V::value_type>
    static constexpr void uninitialize_copy_all(T * _ptr ,const V & v){
        std::size_t i =0;
        try{
            for ( i = 0 ; const auto & x : v){
                std::construct_at(_ptr+i , x); 
                ++i;
            }
        }catch(...){
            std::destroy(_ptr , _ptr + i);
        }
    }

    static constexpr void uninitialize_move_all(T * _ptr , vector & v) noexcept{
        std::size_t i =0;
        for ( i = 0 ; auto & x : v){
            std::construct_at(_ptr+i , std::move(x)); 
            ++i;
        }
    }

    constexpr void deconstruct_all() noexcept{
        std::destroy_n(begin() , size());
        _n = 0;
    }

    constexpr void move_values_from(vector & v)noexcept{
        _n = v.size();              v._n = 0;
        _n_storage = v.capacity();  v._n_storage = 0;
        _start = v._start;          v._start = nullptr;
    }

private:
    std::size_t _n{0};
    std::size_t _n_storage{0};
    T * _start{nullptr};
};

};

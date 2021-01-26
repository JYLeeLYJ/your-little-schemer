#include <gtest/gtest.h>
#include <variant>
#include <optional>
#include "constexpr_containers.hpp"

using cexpr::vector;
using cexpr::box;
using cexpr::cow;

constexpr auto test_const(){
    int n = 0;
    vector v{1,2,3,4};
    n = v.size();
    return n;
}

constexpr auto test_cv(){
    vector v{1,2};
    v.push_back(3);
    v.push_back(4);
    return v;
}

constexpr auto test_cv_copy(){
    vector v{1,2};
    vector v2{1,2,3,4,5};
    v = v2;

    return v.capacity();
}

constexpr bool test_box_const(){
    box b{1};
    static_assert(std::is_same_v<typename decltype(b)::element_type , int>);
    return *b == 1;
}

static_assert(test_const() == 4);
static_assert(test_cv().size() == 4);
static_assert(test_cv_copy() == 7);
static_assert(test_box_const());

TEST(test_cexpr , test_vector){

    vector<int> v{1,2,3,4};
    EXPECT_EQ(v.size() , 4);
    v.push_back(5);
    v.emplace_back(6);
    for(auto i = 1 ; auto x : v){
        EXPECT_EQ(x , i);
        ++i;
    }

    EXPECT_EQ(v.capacity() , 6);
    auto v2 = v;

    EXPECT_EQ(v2.size() , v.size());
    EXPECT_EQ(v2.capacity() , v.capacity());
    
    for(auto i = 1 ; auto x : v2){
        EXPECT_EQ(x , i);
        ++i;
    }

    v = std::move(v);
    EXPECT_EQ(v.size() , 6);
    EXPECT_EQ(v.capacity() , 6);

    vector<int> v3{1};
    v3.push_back(1);
    EXPECT_EQ(v3.size() , 2);

    vector<int> v4{};

    auto v5 = v4;
    EXPECT_EQ(v5.size() , 0);
    EXPECT_EQ(v4.capacity() , v5.capacity());
    EXPECT_EQ(v4.capacity() , 0);
    EXPECT_EQ(v5.begin() , nullptr);
}

TEST(test_cexpr , test_box){
    box b{1};   
    EXPECT_TRUE(b);
    EXPECT_EQ(*b , 1);
}

TEST(test_cexpr , test_cow){
    cow c1{1};
    EXPECT_EQ(c1.cnt() , 1);
    EXPECT_EQ(c1.ref() , 1);
    cow c2 = c1;
    EXPECT_EQ(std::addressof(c1.ref()) , std::addressof(c2.ref()));
    EXPECT_EQ(c1.cnt() , 2);

    auto & c3 = c2;
    EXPECT_EQ(c3.cnt() , 2);

    auto c4 = c3.clone() ;
    EXPECT_EQ(c4.cnt() ,1 );

    c3.into_owned();
    EXPECT_EQ(c1.cnt() , 1);
    EXPECT_EQ(c3.cnt() , 1);

    auto c5 = c3;
    EXPECT_EQ(c5.cnt() , 2);
    c5.mut() = 4;
    EXPECT_EQ(c5.ref() , 4);
    EXPECT_EQ(c5.cnt() , 1);
    EXPECT_EQ(c3.cnt() , 1);

    EXPECT_EQ(c3 , c1);

    cow n{1};
    auto before = &n.ref();
    auto eval = [](cow<int> & v){v.mut(); return v;};
    n = eval(n);
    EXPECT_EQ(n.cnt() , 1);
    EXPECT_EQ(&n.ref() , before);

    struct foo{
        int i;
        foo(int n):i(n){}
        foo(const foo & ) = default;
        foo & operator=(const foo &) = default;
        bool operator== (const foo & ) const = default;
        ~foo(){i=0;}
    };

    using var = std::variant<cow<foo>, foo>;
    var a{cow{foo{1}}};
    ASSERT_EQ(std::get<cow<foo>>(a).cnt() ,1);
    a = std::get<cow<foo>>(a).mut();

    ASSERT_TRUE(std::holds_alternative<foo>(a));
    ASSERT_EQ(std::get<foo>(a) , foo{1});
}
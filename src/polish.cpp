#include <variant>
#include <stdexcept>
#include <numeric>
#include <ranges>
#include <fmt/format.h>

#include "lispy.h"
#include "parsec.h"

namespace{

constexpr int to_int(std::string_view nums){
    auto v = nums | std::ranges::views::transform([](char ch)->int {return ch - '0';}) ;
    return std::accumulate(v.begin() , v.end(), 0 , [](int init , int i ){return init * 10 + i;});
}

constexpr int eval_lispy(char op , cexpr::vector<int> ints){
    switch (op){
    case '+': return std::accumulate(ints.begin() , ints.end() , 0 , std::plus<int>{});
    case '*': return std::accumulate(ints.begin() , ints.end() , 1 , std::multiplies<int>{});
    case '-': 
        if(ints.size() == 1)return -ints[0];
        else if(ints.size() != 2) throw std::invalid_argument{R"( operator '-' needs 2 operands)"};
        return ints[0] - ints[1];
    case '/':
        if(ints.size()!=2) throw std::invalid_argument{R"( operator '/' needs 2 operands)"};
        if(ints[1] == 0) throw std::invalid_argument{"divide by 0 error."};
        return ints[0] / ints[1];
    default : throw std::domain_error{fmt::format("unexcept operator {}." , op)};
    }
}

/// polish parsers

using namespace pscpp;

auto sub_expr(parser_string str)->parser_result<int>; 

auto natural    = fmap(to_int , chars(digit));
auto negative   = fmap(std::negate<int>{} , ('-'_char >> natural)) ;

auto numbers    = negative | natural;
auto opt        = spaces >> one_of("+-*/") << spaces;
auto expr       = spaces >> (numbers | sub_expr) << spaces;
auto lispy      = fmap(eval_lispy , opt , many(expr));
auto _sub_expr  = '('_char >> lispy << ')'_char;

auto polish     = (expr | lispy) << eof ;

auto sub_expr(parser_string str)->parser_result<int>{
    return _sub_expr(str);
}

}

/// parser interface impl
auto parse_polish(std::string_view str) -> std::optional<int>{
    auto result = polish(str);
    if(!result) return {};
    return result->first;
}

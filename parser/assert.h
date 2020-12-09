#include "basic.h"

/* test assert */

static_assert(is_parser_result<parser_result<int>>());
static_assert(!is_parser_result<int>());
static_assert(parser_traits<decltype(result(1))>{});
static_assert(std::is_same_v<parser_traits<decltype(result(1))>::type , int>);

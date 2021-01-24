#pragma once

#include <exception>

namespace lispy{

using exception_base = std::logic_error;

struct parse_error : exception_base{
    using exception_base::exception_base;
};

struct bad_syntax : exception_base {
    using exception_base::exception_base;
};

struct type_error : exception_base{
    using exception_base::exception_base;
};

struct runtime_error : exception_base{
    using exception_base::exception_base;
};

struct internal_error : std::runtime_error{
    using std::runtime_error::runtime_error;
};

}

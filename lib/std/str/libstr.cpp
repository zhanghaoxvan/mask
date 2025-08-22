#include "libstr.hpp"

str::str(const char* s) : s(s) { }

const char* str::toCharArray() { return s; }

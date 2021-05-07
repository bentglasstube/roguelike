#pragma once

#ifdef NDEBUG

class NullLog {};
static NullLog null_log;

template <typename T>
NullLog& operator<<(NullLog& l, T const&) { return l; }
#define DEBUG_LOG null_log

#else

#include <iostream>
#define DEBUG_LOG std::cerr

#endif

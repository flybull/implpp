#include "main.hpp"

static Initializer::unit __unit1( "2.cpp", { "1.cpp" },
    []() { return true; }
);
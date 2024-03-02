#pragma once

#include "stdlibs.hpp"

struct Buffer {
    typedef std::array<char, 4096> Block;
    std::list<Block> data;
};

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */

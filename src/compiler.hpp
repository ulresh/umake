#pragma once

#include "stdlib.hpp"
#include "buffer.hpp"

struct Compiler {
	Compiler(const std::string &cmd, std::list<std::string> args);
	bp::child child;
	Buffer out, err;
};

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */

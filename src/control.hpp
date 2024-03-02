#pragma once

#include "stdlibs.hpp"
#include "compiler.hpp"

struct Control {
	io_service ios;
	std::map<pid_t, std::unique_ptr<Compiler> > compilers;
};

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */

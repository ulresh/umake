#pragma once

#include "stdlibs.hpp"
#include "buffer.hpp"

struct Control;

struct Compiler {
	Compiler(Control &control, const std::string &cmd,
			 std::list<std::string> args);
	Control &control;
	bp::async_pipe pout, perr;
	bp::child child;
	Buffer bout, berr;
	bool eof_out = false, eof_err = false;
};

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */

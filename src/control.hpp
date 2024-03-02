#pragma once

#include "stdlibs.hpp"
#include "compiler.hpp"

struct Control {
	void start(const std::string &cmd, std::list<std::string> args);
	io_service ios;
	bool error = false;
	std::map<pid_t, std::unique_ptr<Compiler> > compilers;
};

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */

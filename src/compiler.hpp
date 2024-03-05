#pragma once

#include "stdlibs.hpp"
#include "buffer.hpp"

struct Control;

struct Compiler {
	Compiler(Control &control, const std::string &source,
			 std::time_t object_mtime, const std::string &dependencies,
			 const std::string &cmd, std::list<std::string> args);
	void async_start_pipes();
	void handle_pipe(bp::async_pipe *pipep, Buffer *bufp,
					 const error_code &ec, std::size_t size);
	static bool check_dependencies(std::time_t object_mtime,
								   const std::string &dependencies);
	Control &control;
	const std::string source, dependencies;
	std::time_t object_mtime;
	std::string cmd;
	std::list<std::string> args;
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

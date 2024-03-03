#include "compiler.hpp"

Compiler::Compiler(Control &control, const std::string &cmd,
				   std::list<std::string> args)
	: control(control)
	, child(bp::exe=cmd, bp::args=args, bp::std_out>pout, bp::std_err>perr)
{}

void Compiler::async_start_pipes() {
}

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */

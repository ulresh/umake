#include "control.hpp"

void Control::start(const std::string &source,
			std::time_t object_mtime, const std::string &dependencies,
			const std::string &cmd, std::list<std::string> args) {
	cout << cmd;
	for(auto &&a : args) cout << ' ' << a;
	cout << endl;
	Compiler *p;
	std::unique_ptr<Compiler>
		h(p = new Compiler(*this, source, object_mtime, dependencies,
						   cmd, args));
	compilers[p->child.id()].reset(h.release());
	p->async_start_pipes();
}

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */

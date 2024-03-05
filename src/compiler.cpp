#include "control.hpp"
#include "compiler.hpp"

Compiler::Compiler(Control &control, const std::string &source,
				   std::time_t source_mtime, const std::string &dependencies,
				   const std::string &cmd, std::list<std::string> args)
	: control(control), source(source)
	, source_mtime(source_mtime), dependencies(dependencies)
	, pout(control.ios), perr(control.ios)
	, child(bp::exe=cmd, bp::args=args, bp::std_out>pout, bp::std_err>perr)
{
	if(!dependencies.empty()) { this->cmd = cmd; this->args = args; }
}

void Compiler::async_start_pipes() {
	bout.data.emplace_back();
	ai::async_read(pout, ai::buffer(bout.data.back()),
				   boost::bind(&Compiler::handle_pipe, this, &pout, &bout,
							   ph::error, ph::bytes_transferred));
	berr.data.emplace_back();
	ai::async_read(perr, ai::buffer(berr.data.back()),
				   boost::bind(&Compiler::handle_pipe, this, &perr, &berr,
							   ph::error, ph::bytes_transferred));
}

void Compiler::handle_pipe(bp::async_pipe *pipep, Buffer *bufp,
						   const error_code &ec, std::size_t size) {
	cout << source << (pipep == &pout ? " out" : " err")
		 << " size:" << size << " ec:" << ec << endl;
	bufp->last_block_size += size;
	if(ec) {
		if(pipep == &pout) eof_out = true; else eof_err = true;
		if(eof_out && eof_err) {
			ptime t = microsec_clock::local_time();
			child.wait();
			int result = child.exit_code();
			cout << source << " result:" << result << " wait time:"
				 << (microsec_clock::local_time() - t) << endl;
			if(!berr.empty()) {
				cerr << berr << endl;
				if(!berr.eol_at_end()) cerr << endl;
			}
			if(!bout.empty()) {
				cout << bout << endl;
				if(!bout.eol_at_end()) cout << endl;
			}
			if(result) control.error = true;
			else if(!dependencies.empty() &&
					check_dependencies(source_mtime, dependencies)) {
				args.pop_front(); // -E
				args.pop_front(); // -MM
				args.pop_front(); // -MF
				args.pop_front(); // dependencies
				args.pop_front(); // -MT
				control.build = true;
				args.emplace_front("-o");
				args.emplace_front("-c");
				control.start(source, 0, std::string(), cmd, args);
			}
			control.compilers.erase(child.id());
		}
	}
	else {
		auto &back = bufp->data.back();
		if(bufp->last_block_size < back.size())
			ai::async_read(*pipep,
						   ai::buffer(back.data() + bufp->last_block_size,
									  back.size() - bufp->last_block_size),
					   boost::bind(&Compiler::handle_pipe, this, pipep, bufp,
								   ph::error, ph::bytes_transferred));
		else {
			bufp->data.emplace_back();
			ai::async_read(*pipep, ai::buffer(bufp->data.back()),
					   boost::bind(&Compiler::handle_pipe, this, pipep, bufp,
								   ph::error, ph::bytes_transferred));
		}
	}
}

inline int buffer_size() { return 4096; }
struct FileCloser {
	FileCloser(int file) : file(file) {}
	~FileCloser() { auto ret = close(file); }
	int file;
};

bool Compiler::check_dependencies(std::time_t source_mtime,
								  const std::string &dependencies) {
	FileCloser file(open(dependencies.c_str(), O_RDONLY));
	if(file.file < 0) {
		cerr << "Ошибка открытия для чтения файла зависимостей "
			 << dependencies << endl;
		return true;
	}
	std::string cache;
	char *buffer;
	std::unique_ptr<char[]> buffer_holder(buffer = new char[buffer_size()]);
	for(;;) {
		int size = read(file.file, buffer, buffer_size());
		cout << dependencies << " size:" << size << endl;
		if(size < 0) {
			cerr << "Ошибка чтения файла зависимостей "
				 << dependencies << endl;
			return true;
		}
		else if(size == 0) {
			// TODO
			break;
		}
		else {
			// TODO
		}
	}
	return true; // TODO изменить в итоговой версии
}

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */

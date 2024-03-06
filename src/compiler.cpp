#include "control.hpp"
#include "compiler.hpp"

Compiler::Compiler(Control &control, const std::string &source,
				   std::time_t object_mtime, const std::string &dependencies,
				   const std::string &cmd, std::list<std::string> args)
	: control(control), source(source)
	, object_mtime(object_mtime), dependencies(dependencies)
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
	ulog << source << (pipep == &pout ? " out" : " err")
		 << " size:" << size << " ec:" << ec << endl;
	bufp->last_block_size += size;
	if(ec) {
		if(pipep == &pout) eof_out = true; else eof_err = true;
		if(eof_out && eof_err) {
			ptime t = microsec_clock::local_time();
			child.wait();
			int result = child.exit_code();
			ulog << source << " result:" << result << " wait time:"
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
					check_dependencies(object_mtime, dependencies)) {
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

/*
 \
s0 s1 s0
/home/reshu/project/umake/obj/name test.o: \
s0 s2                                    s3 s4 s5
 /home/reshu/project/umake/src/name\ test.cpp \
s4 s6                               s7        s4 s5
 /home/reshu/project/umake/src/name\ test.hpp \
 /home/reshu/project/umake/src/stdlibs.hpp
                                           s8
 */

bool Compiler::check_dependencies(std::time_t object_mtime,
								  const std::string &dependencies) {
	FileCloser file(open(dependencies.c_str(), O_RDONLY));
	if(file.file < 0) {
		cerr << "Ошибка открытия для чтения файла зависимостей "
			 << dependencies << endl;
		return true;
	}
	int state = 0;
	std::string filename;
	char *buffer;
	bool skip = true; // первым указан source_file, который уже проверен
	std::unique_ptr<char[]> buffer_holder(buffer = new char[buffer_size()]);
	for(;;) {
		int size = read(file.file, buffer, buffer_size());
		ulog << dependencies << " size:" << size << endl;
		if(size < 0) {
			cerr << "Ошибка чтения файла зависимостей "
				 << dependencies << endl;
			return true;
		}
		else if(size == 0) {
			if(state != 8 || !filename.empty()) goto bad_format;
			break;
		}
		else {
			char *ptr = buffer, *mark = buffer, *end = buffer + size;
			for(; ptr < end; ++ptr) switch(state) {
				default: goto bad_format;
				case 0: switch(*ptr) {
					case ' ': break;
					case '\\': ++state; break;
					case '\n': goto bad_format;
					default: state = 2;
				}
				break;
				case 1:
				case 5:
					if(*ptr != '\n') goto bad_format;
					--state;
					break;
				case 2: switch(*ptr) {
					// case ' ': // in default
					case '\n':
					case '\\': goto bad_format;
					case ':': ++state; break;
					default: ;
					}
					break;
				case 3: switch(*ptr) {
					case ' ': ++state; break;
					// case ':': // in default
					case '\\':
					case '\n': goto bad_format;
						// TODO
					default: --state;
					}
					break;
				case 4: switch(*ptr) {
					case ' ': break;
					case '\\': ++state; break;
					case '\n': goto bad_format;
					default: mark = ptr; state = 6;
					}
					break;
				case 6: switch(*ptr) {
					case '\n':
						state = 10;
					case ' ':
						state -= 2;
						if(ptr > mark) filename.append(mark, ptr - mark);
						if(skip) {
							skip = false;
							ulog << dependencies << " skip "
								 << filename << endl;
						}
						else {
							if(fs::last_write_time(filename) >=
							   object_mtime) {
								ulog << dependencies << " need build for "
									 << filename << ' '
							 << from_time_t(fs::last_write_time(filename))
							 << " object:" << from_time_t(object_mtime)
									 << endl;
								return true;
							}
							ulog << dependencies << ' ' << filename << endl;
						}
						filename.clear();
						break;
					case '\\':
						if(ptr > mark) filename.append(mark, ptr - mark);
						++state;
						break;
					default: ;
					}
					break;
				case 7:
					if(*ptr != ' ') goto bad_format;
					mark = ptr; --state;
					break;
			}
			if(state == 6 && mark < end) filename.append(mark, end - mark);
		}
	}
	return false;
 bad_format:
	cerr << "Ошибка формата в файле зависимостей " << dependencies
		 << " state:" << state << endl;
	return true;
}

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */

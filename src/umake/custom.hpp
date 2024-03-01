#include <list>
#include <string>

struct Custom {
	void add_include_path(const std::string &p) {
		include_pathes.push_back(s); }
	void add_library(const std::string &p) {
		libraries.push_back(s); }
	void add_library_file(const std::string &p) {
		library_files.push_back(s); }
	std::list<std::string> include_pathes, libraries, library_files;
};

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */

#include <list>
#include <string>

struct Custom {
	void add_include_path(const std::string &s) {
		include_pathes.push_back(s); }
	void add_library(const std::string &s) {
		libraries.push_back(s); }
	void add_library_file(const std::string &s) {
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

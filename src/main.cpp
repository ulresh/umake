#include <iostream>
#include <boost/filesystem.hpp>

using std::cout;
using std::cerr;
using std::endl;
using std::flush;
namespace fs = boost::filesystem;

bool has_git_folder(fs::path folder) {
	folder /= ".git";
	return fs::exists(folder);
}

int main(int argc, const char **argv) {
	fs::path folder = fs::current_path();
	cout << "folder:" << folder << endl;
	if(has_git_folder(folder)) {
		cerr << "Невозможен запуск в корне проекта" << endl;
		return 1;
	}
	cout << "filename:" << folder.filename() << endl;
	cout << "parent:" << folder.parent_path() << endl;
	fs::path up = folder.filename();
	bool in_src = up == "src";
	for(;;) {
		folder = folder.parent_path();
		if(folder.empty()) {
			cerr << "Не получилось найти папку .git выше текущей папки"
				 << endl;
			return 1;
		}
		if(has_git_folder(folder)) break;
		fs::path n = up;
		up = folder.filename();
		in_src = up == "src";
		up /= n;
	}
	cout << "project:" << folder << " source:" << up
		 << " in_src:" << in_src << endl;
	return 0;
}

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */

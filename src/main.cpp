#include <iostream>
#include <boost/filesystem.hpp>

using std::cout;
using std::cerr;
using std::endl;
using std::flush;
namespace fs = boost::filesystem;
using boost::system::error_code;

struct RootFolder {
	static bool has_git_folder(fs::path folder) {
		folder /= ".git";
		return fs::exists(folder);
	}
	RootFolder() : current(fs::current_path()) {
		if(has_git_folder(current)) {
			cerr << "Невозможен запуск в корне проекта" << endl;
			return;
		}
		delta = current.filename();
		in_src = delta == "src";
		root = current;
		for(;;) {
			root = root.parent_path();
			if(root.empty()) {
				cerr << "Не получилось найти папку .git выше текущей папки"
					 << endl;
				return;
			}
			if(has_git_folder(root)) break;
			fs::path n = delta;
			delta = root.filename();
			in_src = delta == "src";
			delta /= n;
		}
		valid = true;
	}
	bool valid = false, in_src;
	fs::path current, root, delta;
};

int main(int argc, const char **argv) {
	RootFolder root_folder;
	if(!root_folder.valid) return 1;
	cout << "folder:" << root_folder.current << endl;
	cout << "project:" << root_folder.root << " source:" << root_folder.delta
		 << " in_src:" << root_folder.in_src << endl;
	for(auto &&file :
			fs::recursive_directory_iterator(root_folder.current))
		if(!file.is_directory() && file.path().extension() == ".cpp" &&
		   file.path().filename().string()[0] != '.') {
			cout << file << endl;
		}
	return 0;
}

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */

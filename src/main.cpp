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
		root = current;
		bool in_src = root.filename() == "src";
		for(;;) {
			root = root.parent_path();
			if(root.empty()) {
				cerr << "Не получилось найти папку .git выше текущей папки"
					 << endl;
				return;
			}
			if(has_git_folder(root)) break;
			in_src = root.filename() == "src";
		}
		base = root;
		if(in_src) base /= "src";
		valid = true;
	}
	fs::path object_file(const fs::path &source_file) const {
		auto delta = source_file.string().substr(base.size() + 1);
		cout << delta << endl;
		fs::path result(root);
		result /= "obj";
		result /= delta;
		result.replace_extension(".o");
		fs::path folder(result);
		folder.remove_filename();
		if(!fs::is_directory(folder)) {
			cout << "mkdir " << folder << endl;
			fs::create_directories(folder);
		}
		return result;
	}
	bool valid = false;
	fs::path current, root, base;
};

int main(int argc, const char **argv) {
	RootFolder root_folder;
	if(!root_folder.valid) return 1;
	cout << "folder:" << root_folder.current << endl;
	cout << "project:" << root_folder.root
		 << " base:" << root_folder.base << endl;
	for(auto &&file :
			fs::recursive_directory_iterator(root_folder.current))
		if(!file.is_directory() && file.path().extension() == ".cpp" &&
		   file.path().filename().string()[0] != '.') {
			auto object_file = root_folder.object_file(file);
			cout << file << " -> " << object_file << endl;
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

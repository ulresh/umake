#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <boost/dll.hpp>

#include "umake/custom.hpp"

using std::cout;
using std::cerr;
using std::endl;
using std::flush;
namespace fs = boost::filesystem;
namespace bp = boost::process;
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
	fs::path binary_file() const {
		fs::path result(root);
		result /= "bin";
		if(!fs::is_directory(result)) {
			uout << "mkdir " << result << endl;
			fs::create_directory(result);
		}
		if(current == base) result /= root.filename();
		else result /= current.filename();
		return result;
	}
	bool valid = false;
	fs::path current, root, base;
	bp::filesystem::path cc;
};

void load_custom_file(Custom &custom, RootFolder &root_folder,
					  const char *filename) {
	fs::path source_file = root_folder.current;
	source_file /= filename;
	if(fs::exists(source_file)) {
		auto object_file = root_folder.object_file(source_file);
		object_file.replace_extension(".umake.so");
		cout << source_file << " -> " << object_file << endl;
		std::string inc;
		{	auto env = boost::this_process::environment();
			auto inc_env = env["UMAKE_CUSTOM_INCLUDE_PATH"];
			if(inc_env.empty()) {
#ifdef UMAKE_DEFAULT_CUSTOM_INCLUDE_PATH
				inc = "-I" #UMAKE_DEFAULT_CUSTOM_INCLUDE_PATH;
#else
				inc = "-I../src0";
#endif
			}
			else {
				inc = "-I";
				inc.append(inc_env.to_string());
				cout << "inc:" << inc << endl;
			}
		}
		int result = bp::system(root_folder.cc, "-x", "c++", "-shared",
								inc,
								"-o", object_file.string(),
								source_file.string());
		cout << "result:" << result << endl;
		if(result != 0) exit(1);
		boost::dll::shared_library lib(object_file.string());
		boost::function<void(Custom&)> load;
		try { load = lib.get<void(Custom&)>("_Z11build_umakeR6Custom"); }
		catch(const std::exception &e) {
			cerr << "exception: " << e.what() << endl; }
		if(!load) { cerr << "Не найдена функция build_umake в файле "
						 << source_file << endl; exit(1); }
		load(custom);
		lib.unload();
	}
}

void load_custom(Custom &custom, RootFolder &root_folder) {
	load_custom_file(custom, root_folder, "build.umake");
	load_custom_file(custom, root_folder, "build.local.umake");
}

int main(int argc, const char **argv) {
	RootFolder root_folder;
	if(!root_folder.valid) return 1;
	cout << "folder:" << root_folder.current << endl;
	cout << "project:" << root_folder.root
		 << " base:" << root_folder.base << endl;
	root_folder.cc = bp::search_path("g++");
	cout << "cc:" << root_folder.cc << endl;
	Custom custom;
	load_custom(custom, root_folder);
	if(!custom.include_pathes.empty())
		cout << "custom.include_pathes[0]:" << custom.include_pathes.front()
			 << endl;
	std::list<std::string> ldargs;
	for(auto &&file :
			fs::recursive_directory_iterator(root_folder.current))
		if(!file.is_directory() && file.path().extension() == ".cpp" &&
		   file.path().filename().string()[0] != '.') {
			auto object_file = root_folder.object_file(file);
			cout << file << " -> " << object_file << endl;
			std::list<std::string> ccargs;
			ccargs.emplace_back("-c");
			ccargs.emplace_back("-o");
			ccargs.push_back(object_file.string());
			ccargs.push_back(file.path().string());
			for(auto &&p : custom.system_include_pathes) {
				ccargs.emplace_back("-isystem");
				ccargs.push_back(p);
			}
			for(auto &&p : custom.include_pathes)
				ccargs.push_back(std::string("-I")+p);
			cout << root_folder.cc;
			for(auto &&a : ccargs) cout << ' ' << a;
			cout << endl;
			int result = bp::system(bp::exe=root_folder.cc, bp::args=ccargs);
			cout << "result:" << result << endl;
			if(result != 0) exit(1);
			ldargs.push_back(object_file.string());
		}
	auto binary_file = root_folder.binary_file();
	cout << "binary:" << binary_file << endl;
	ldargs.push_front(binary_file.string());
	ldargs.push_front("-o");
	for(auto &&p : custom.library_files) ldargs.push_back(p);
	for(auto &&p : custom.libraries) ldargs.push_back(std::string("-l")+p);
	cout << root_folder.cc;
	for(auto &&a : ldargs) cout << ' ' << a;
	cout << endl;
	int result = bp::system(bp::exe=root_folder.cc, bp::args=ldargs);
	cout << "result:" << result << endl;
	return result;
}

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */

#include "umake/custom.hpp"
#include "stdlibs.hpp"
#include "root-folder.hpp"

void load_custom_file(Custom &custom, RootFolder &root_folder,
					  const char *filename) {
	fs::path source_file = root_folder.current;
	source_file /= filename;
	if(fs::exists(source_file)) {
		auto object_file = root_folder.object_file(source_file);
		object_file.replace_extension(".umake.so");
		cout << source_file << " -> " << object_file << endl;
		int result = bp::system(root_folder.cc, "-x", "c++", "-shared",
								"-I.",
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
	cout << "cpu:" << std::thread::hardware_concurrency() << endl;
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

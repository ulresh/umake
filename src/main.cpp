#include "umake/custom.hpp"
#include "stdlibs.hpp"
#include "root-folder.hpp"
#include "control.hpp"

std::ofstream ulog;
TeeDevice uout_dev(cout,ulog);
boost::iostreams::stream<TeeDevice> uout(uout_dev);

void load_custom_file(Custom &custom, RootFolder &root_folder,
					  const char *filename) {
	fs::path source_file = root_folder.current;
	source_file /= filename;
	if(fs::exists(source_file)) {
		auto object_file = root_folder.object_file(source_file);
		object_file.replace_extension(".umake.so");
		uout << source_file << " -> " << object_file << endl;
		std::string inc;
		{	auto env = boost::this_process::environment();
			auto inc_env = env["UMAKE_CUSTOM_INCLUDE_PATH"];
			if(inc_env.empty()) inc = "-I.";
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
		ulog << "result:" << result << endl;
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
	cout << "https://github.com/ulresh/umake" << endl;
	ulog.open("umake.log", std::ofstream::trunc);
	RootFolder root_folder;
	if(!root_folder.valid) return 1;
	ulog << "folder:" << root_folder.current << endl;
	ulog << "project:" << root_folder.root
		 << " base:" << root_folder.base << endl;
	root_folder.cc = bp::search_path("g++");
	ulog << "cc:" << root_folder.cc << endl;
	ulog << "cpu:" << std::thread::hardware_concurrency() << endl;
	Custom custom;
	load_custom(custom, root_folder);
	std::list<std::string> ldargs;
	Control control;
	for(auto &&file :
			fs::recursive_directory_iterator(root_folder.current))
		if(!file.is_directory() && file.path().extension() == ".cpp" &&
		   file.path().filename().string()[0] != '.') {
			auto object_file = root_folder.object_file(file);
			ulog << file << " -> " << object_file << endl;
			ldargs.push_back(object_file.string());
			bool build = false;
			std::string dependencies;
			std::time_t object_mtime = 0;
			if(!fs::exists(object_file)) build = true;
			else {
				std::time_t source_mtime = fs::last_write_time(file);
				object_mtime = fs::last_write_time(object_file);
				if(source_mtime >= object_mtime) {
					build = true;
					ulog << "source:" << from_time_t(source_mtime)
						 << " object:" << from_time_t(object_mtime)
						 << endl;
				}
				else {
					auto dependencies_file = object_file;
					dependencies_file.replace_extension(".dep");
					if(!fs::exists(dependencies_file) || source_mtime >=
					   fs::last_write_time(dependencies_file)) {
						dependencies = dependencies_file.string();
						ulog << "source:" << from_time_t(source_mtime)
		<< " dep:" << from_time_t(fs::last_write_time(dependencies_file))
							 << endl;
					}
					else if(Compiler::check_dependencies(object_mtime,
								dependencies_file.string())) build = true;
				}
			}
			if(!build && dependencies.empty()) continue;
			std::list<std::string> ccargs;
			if(build) {
				control.build = true;
				ccargs.emplace_back("-c");
				ccargs.emplace_back("-o");
			}
			else {
				ccargs.emplace_back("-E");
				ccargs.emplace_back("-MM");
				ccargs.emplace_back("-MF");
				ccargs.push_back(dependencies);
				ccargs.emplace_back("-MT");
			}
			ccargs.push_back(object_file.string());
			ccargs.push_back(file.path().string());
			ccargs.emplace_back("-fdiagnostics-color");
			for(auto &&p : custom.system_include_pathes) {
				ccargs.emplace_back("-isystem");
				ccargs.push_back(p);
			}
			for(auto &&p : custom.include_pathes)
				ccargs.push_back(std::string("-I")+p);
			control.start(file.path().string().substr(
								root_folder.current.size() + 1),
						  object_mtime, dependencies,
						  root_folder.cc.string(), ccargs);
			if(control.compilers.size() >=
			   std::thread::hardware_concurrency()) {
				uout << flush;
				do { control.ios.run_one(); }
				while(control.compilers.size() >=
					  std::thread::hardware_concurrency());
			}
			if(control.error) break;
		}
	control.ios.run();
	if(control.error) exit(1);
	auto binary_file = root_folder.binary_file();
	uout << "binary:" << binary_file << endl;
	if(!control.build && fs::exists(binary_file)) return 0;
	ldargs.push_front(binary_file.string());
	ldargs.push_front("-o");
	for(auto &&p : custom.library_files) ldargs.push_back(p);
	for(auto &&p : custom.libraries) ldargs.push_back(std::string("-l")+p);
	uout << root_folder.cc;
	for(auto &&a : ldargs) uout << ' ' << a;
	uout << endl;
	int result = bp::system(bp::exe=root_folder.cc, bp::args=ldargs);
	ulog << "result:" << result << endl;
	return result;
}

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */

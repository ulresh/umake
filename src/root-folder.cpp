#include "root-folder.hpp"

RootFolder::RootFolder() : current(fs::current_path()) {
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

fs::path RootFolder::object_file(const fs::path &source_file) const {
	auto delta = source_file.string().substr(base.size() + 1);
	ulog << delta << endl;
	fs::path result(root);
	result /= "obj";
	result /= delta;
	result.replace_extension(".o");
	fs::path folder(result);
	folder.remove_filename();
	if(!fs::is_directory(folder)) {
		uout << "mkdir " << folder << endl;
		fs::create_directories(folder);
	}
	return result;
}

fs::path RootFolder::binary_file() const {
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

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */

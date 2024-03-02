#pragma once

#include "stdlibs.hpp"

struct RootFolder {
	static bool has_git_folder(fs::path folder) {
		folder /= ".git";
		return fs::exists(folder);
	}
	RootFolder();
	fs::path object_file(const fs::path &source_file) const;
	fs::path binary_file() const;
	bool valid = false;
	fs::path current, root, base;
	bp::filesystem::path cc;
};

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */

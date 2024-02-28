#include <iostream>
#include <boost/filesystem.hpp>

using std::cout;
using std::endl;
using std::flush;
namespace fs = boost::filesystem;

int main(int argc, const char **argv) {
	cout << "started" << endl << flush;
	fs::path folder = fs::current_path();
	cout << "folder:" << folder.c_str() << endl << flush;
	fs::path filename = folder.filename();
	cout << "filename:" << filename.c_str() << endl << flush;
	// for(auto ptr = folder.begin(), end = folder.end(); ptr != end; ++ptr)
	// 	cout << "\t* " << ptr->c_str() << endl << flush;
	// for(auto &&ptr : folder)
	// 	cout << "\t+ " << ptr.c_str() << endl << flush;
	// fs::path rbegin = *folder.rbegin();
	// cout << "rbegin:" << rbegin.c_str() << endl << flush;
	// fs::path rend = *folder.rend();
	// cout << "rend:" << rend.c_str() << endl << flush;
	// for(auto ptr = folder.rbegin(), end = folder.rend(); ptr != end; ++ptr)
	// 	cout << "\t* " << ptr->c_str() << endl << flush;
	return 0;
}

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */

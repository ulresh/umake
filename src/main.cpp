#include <iostream>
#include <boost/filesystem.hpp>

using std::cout;
using std::endl;
using std::flush;
namespace fs = boost::filesystem;

int main(int argc, const char **argv) {
	fs::path folder = fs::current_path();
	cout << "folder:" << folder << endl;
	cout << "filename:" << folder.filename() << endl;
	for(auto ptr = folder.rbegin(), end = folder.rend(); ptr != end; ++ptr)
		cout << "\t* " << ptr->c_str() << endl;
	return 0;
}

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */

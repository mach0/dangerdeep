// Danger from the Deep (C)+(W) Thorsten Jordan. SEE LICENSE
//
// system dependent main
//
// WIN32: use WinMain, divide cmd line to list of strings
// UNIX: C-like main, make strings from char** array
// MacOSX: ? fixed with objective C code ?
//

#include <list>
#include <string>
using namespace std;

int mymain(list<string>& args);

#ifdef WIN32

int WinMain(HINSTANCE, HINSTANCE, LPSTR cmdline, int)
{
	string mycmdline(cmdline);
	list<string> args;
	// parse mycmdline
	while (mycmdline.length() > 0) {
		string::size_type st = mycmdline.find(" ");
		args.push_back(mycmdline.substr(0, st));
		if (st == string::npos) break;
		mycmdline = mycmdline.substr(st+1);
	}
	return mymain(args);
}

#else	// UNIX

int main(int argc, char** argv)
{
	list<string> args;
	//parse argc, argv, do not store program name
	while (argc > 1) {
		args.push_front(string(argv[--argc]));
	}
	return mymain(args);
}

#endif
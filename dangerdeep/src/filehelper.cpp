// file helper functions
// (C)+(W) Thorsten Jordan. SEE LICENSE

#include "filehelper.h"
#include <vector>

#ifdef WIN32
static WIN32_FIND_DATA Win32_FileFind_Temporary;
static bool Win32_FileFind_Temporary_used = false;
directory open_dir(const string& filename)
{
	directory d;
	d.temporary_used = true;
	FindFirstFile(filename.c_str(), &d.Win32_FileFind_Temporary);
	return d;
}
string read_dir(directory d)
{
	if (d.temporary_used) {
		d.temporary_used = false;
		return d.Win32_FileFind_Temporary.cFileName;
	} else {
		BOOL b = FindNextFile(d.dir, &d.Win32_FileFind_Temporary);
		if (b == TRUE)
			return string(Win32_FileFind_Temporary.cFileName);
		else
			return "";
	}
}
void close_dir(directory d)
{
	FindClose(d.dir);
}
bool make_dir(const string& dirname)
{
	return (CreateDirectory(dirname.c_str(), NULL) == TRUE);
}
string get_current_directory(void)
{
	unsigned sz = 256;
	vector<char> s(sz);
	DWORD dw = 0;
	while (dw == 0) {
		dw = GetCurrentDirectory(sz, &s[0]);
		if (dw == 0) {
			sz += sz;
			s.resize(sz);
		}
	}
	return string(&s[0]) + PATHSEPARATOR;
}
bool is_directory(const string& filename)
{
	return ((GetFileAttributes(filename.c_str()) & FILE_ATTRIBUTE_DIRECTORY) != 0);
}

#else	/* Win32 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
directory open_dir(const string& filename)
{
	directory d;
	d.dir = opendir(filename.c_str());
	return d;
}
string read_dir(directory d)
{
	struct dirent* dir_entry = readdir(d.dir);
	if (dir_entry) {
		return string(dir_entry->d_name);
	}
	return "";
}
void close_dir(directory d)
{
	closedir(d.dir);
}
bool make_dir(const string& dirname)
{
	int err = mkdir(dirname.c_str(), 0755);
	return (err != -1);
}
string get_current_directory(void)
{
	unsigned sz = 256;
	vector<char> s(sz);
	char* c = 0;
	while (c == 0) {
		c = getcwd(&s[0], sz-1);
		if (!c) {
			sz += sz;
			s.resize(sz);
		}
	}
	return string(&s[0]) + PATHSEPARATOR;
}
bool is_directory(const string& filename)
{
/*
	// better code. has to be tested yet.
	struct stat fileinfo;
	int errno = stat(filename.c_str(), &fileinfo);
	if (errno != 0) return false;
	if (S_ISDIR(fileinfo.st_mode)) return true;
	return false;
*/
	
	DIR* d = opendir(filename.c_str());	//fixme: could this be done smarter?
						// make sure this doesn't create a new directory
	if (d != 0) {
		closedir(d);
		return true;
	}
	return false;
}

#endif /* Win32 */

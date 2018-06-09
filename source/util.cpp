#include <iostream>
#include <string>
#include <switch.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include "util.hpp"


using std::string;
using std::vector;

// Functions 

string getFileExt(const string& fname) {
	size_t ext_pos = fname.rfind('.', fname.length());
	if (ext_pos != string::npos) {
		return(fname.substr(ext_pos+1, fname.length()-ext_pos));
	} else {
		return("");
	}
}

int findFilesByExt(const string& path, const string& ext, vector<string> &out) {
	struct dirent *dp;
	DIR *dfd = opendir(path.c_str());
	if(dfd != NULL) {
		while ((dp = readdir(dfd)) != NULL) {
			string fname = path + "/" + dp->d_name;
			string myext = getFileExt(fname);
						if (myext == ext) {
				out.push_back(fname);
			}
		}
		closedir(dfd);
		return 0;
	} else {
		return errno;
	}
}

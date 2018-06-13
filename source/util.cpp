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

string timeFromSeconds(int seconds) {
	string retval = "";
	int isec = seconds%60;
	int imin = (isec/60)%60;
	int ihr = isec/(60*60)%24;
	int iday = isec/(60*60*24)%365;
	int iyr = isec/(60*60*24*365);
	char tmpbuf[4];
	sprintf(tmpbuf, "%02d", isec);
	retval += tmpbuf;
	if (seconds > 59) {
		sprintf(tmpbuf, "%02d:", imin);
		retval.insert(0, tmpbuf);
	} 
	if (seconds > 3599) {
		sprintf(tmpbuf, "%02d:", ihr);
		retval.insert(0, tmpbuf);
	}
	if (seconds > 86399) {
		sprintf(tmpbuf, "%02d:", iday);
		retval.insert(0, tmpbuf);
	}
	if (seconds > 31535999) {
		sprintf(tmpbuf, "%02d:", iyr);
		retval.insert(0, tmpbuf);
	}
	return retval;
}

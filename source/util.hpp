#ifndef UTIL_H
#define UTIL_H
#include <string>
#include <vector>

using namespace std;

string getFileExt(const string& fname);
int findFilesByExt(const string& path, const string& ext, vector<string> &out);
string timeFromSeconds(int seconds);

		
#endif

#include "stringhelper.h"

vector<string> StringHelper::split(const string &s, char delim, bool ignoreEmpty) {
	vector<string> elems;
	stringstream ss(s);
	string item;
	while (getline(ss, item, delim)) {
		if(ignoreEmpty && item.empty()){
			continue;
		}
		elems.push_back(item);
	}
	return elems;
}

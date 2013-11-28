#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include "image.h"
#include "mesh.h"
#include "stringhelper.h"

using namespace std;

class Reader
{
	private:
		static vector<string> split(const string &s, char delim, bool ignoreEmpty);

	public:
		static bool readObj(const char* name, Mesh* m);
		static void saveObj(const char* name, Mesh* m);
		static bool readMtl(const char* s, Mesh* m);
		static Image* readPpm(string s);
		Group* searchForExistingGroup(string name);

};

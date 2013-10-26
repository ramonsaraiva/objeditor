#include <vector>
#include <string>
#include <sstream>

using namespace std;

class StringHelper
{
    public:
        static vector<string> split(const string &s, char delim, bool ignoreEmpty);
};

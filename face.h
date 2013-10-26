#include <vector>

using namespace std;

class Face{
	private:
		vector<int> verts;
		vector<int> norms;
		vector<int> texts;

	public:
		void addVert(int v);
		void addNorm(int n);
		void addText(int t);

		vector<int> getVerts(void);
		vector<int> getNorms(void);
		vector<int> getTexts(void);
};

#include <string>

class Material{
	private:
		std::string name;
		float diffuse[4];
		float ambient[4];
		float specular[4];
		float shininess;
		std::string textName;
		int id;
	
	public:
		
		static int textCount;
		
		static int getTextCount(void);
		static void setTextCount(int count);
	
		Material(){}
		Material(std::string n);
	
		std::string getName(void);
		float* getDiffuse(void);
		float* getAmbient(void);
		float* getSpecular(void);
		float getShininess(void);
		int getID(void);
		std::string getTextName(void);
		
		void setDiffuse(float r, float g, float b);
		void setAmbient(float r, float g, float b);
		void setSpecular(float r, float g, float b);
		void setShininess(float n);
		void setTextName(std::string tn);
		void setD(float d);
		void setID(int i);
		
		bool hasText(void);
};
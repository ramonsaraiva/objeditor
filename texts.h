
class Texts{
	private:
		float coords[2];

	public:
		Texts(){}
		Texts(float c[2]);
		Texts(float x, float y);
		float* getCoords(void);
};
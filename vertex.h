
class Vertex{
	private:
		float coords[3];

	public:
		Vertex(){}
		Vertex(float c[3]);
		Vertex(float x, float y, float z);
		float* getCoords(void);
};
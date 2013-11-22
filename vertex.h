
class Vertex{
	private:
		float coords[3];
		bool deletable = false;

	public:
		Vertex(){}
		Vertex(float* c);
		Vertex(float x, float y, float z);

		float* getCoords(void);
		bool is_deletable();
		void set_deletable(bool b);
};

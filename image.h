class Image
{
	private:
		int* pixels;
		int width;
		int height;

	public:
		Image();

		void setSize(int w, int h);

		void setPixel(int rgb, int x, int y);
		int getPixel(int x, int y);
		int* getPixels();

		int getWidth();
		int getHeight();

		void loadPPM(const char* src);
};

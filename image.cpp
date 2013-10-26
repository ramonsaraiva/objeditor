#include "image.h"
#include <iostream>
#include <fstream>

using namespace std;

Image::Image() {}

void Image::setSize(int w, int h)
{
	width = w;
	height = h;

	pixels = new int[w * h];
}

void Image::setPixel(int rgb, int x, int y)
{
	pixels[x + y * width] = rgb;
}

int Image::getPixel(int x, int y)
{
	return pixels[x + y * width];
}

int* Image::getPixels()
{
	return pixels;
}

int Image::getWidth()
{
	return width;
}

int Image::getHeight()
{
	return height;
}

void Image::loadPPM(const char* src)
{
	ifstream in;

	const int buffer_size = 100;
	char buffer[buffer_size];

	int cols, rows, max;
	int r, g, b, rgb;

	in.open(src);

	in.getline(buffer, buffer_size);

	if (buffer[0] == 'P' && buffer[1] == '3')
	{
		do
		{
			in.getline(buffer, buffer_size);
		}
		while (buffer[0] == '#');
			
		sscanf(buffer,"%d %d",&cols,&rows);

		in >> max;

		setSize(cols, rows);

		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < cols; j++)
			{
				in >> r >> g >> b;
				rgb = (r << 16) | (g << 8) | b | (255 << 24);

				setPixel(rgb, j, height - i - 1);
			}
		}
	}
}


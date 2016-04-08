#ifndef IMAGE_LOADER_H_INCLUDED
#define IMAGE_LOADER_H_INCLUDED

#include "string"
using namespace std;
//Represents an image
class Image {
	public:
		Image(char* ps, int w, int h);
		~Image();
		
		char* pixels;
		int width;
		int height;
};

//Reads a bitmap image from file.
Image* loadBMP(string filename);


#endif

// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>

//Define some standard BMP values found from standard BMP file structure
#define DATA_OFFSET_OFFSET 0x000A
#define HEIGHT_OFFSET 0x0016
#define WIDTH_OFFSET 0x0012
#define BITS_PER_PIXEL_OFFSET 0x001C
#define COLOR_TABLE_OFFSET 0x0032

void readBmp(const char* filename, char **pixels, int *height, int *width, int *bytesPerPixel, char **colorTable);
void cropBMP(const char* filename, char *pixels, int height, int width, int bytesPerPixel, char **colorTable);
void writeBMP(const char* filename, char *pixels, int height, int width, int bytesPerPixel, char **colorTable);


int main()
{
	//variables to read in then use is write
	char* pixels, *colorTable;
	int height, width, bytesPerPixel;
	//char *temp = readBmp("./GivenImg/fozzie-in.bmp", &pixels, &height, &width, &bytesPerPixel, &colorTable);
	readBmp("./GivenImg/fozzie-in.bmp", &pixels, &height, &width, &bytesPerPixel, &colorTable);
	//cropBMP("./GivenImg/fozzie-in.bmp", pixels, height, width, bytesPerPixel, &colorTable);
	writeBMP("./GivenImg/fozzie-out.bmp", pixels, height, width, bytesPerPixel, &colorTable);

}

void readBmp(const char* filename, char **pixels, int *height, int *width, int *bytesPerPixel, char **colorTable)
{
	//BMPs are separated into a few sections
	//File header: provides information about the file itself
	//Information header: Properties of the pixel data
	//Data: actual pixel data

	std::cout << "Image Cropper\n";
	FILE *streamIn;
	errno_t err;
	//Read in image to crop
	err = fopen_s(&streamIn, "./GivenImg/fozzie-in.bmp", "rb");
	//Make sure image opens properly
	if (err != 0) {
		std::cout << "Error Reading file.";
	}

	//Now we can start extracting data from the file header
	int dataOffset;
	fseek(streamIn, DATA_OFFSET_OFFSET, SEEK_SET);
	fread(&dataOffset, 4, 1, streamIn);
	std::cout << "Data offset: " << dataOffset;

	//Grab the height and width of the image
	fseek(streamIn, HEIGHT_OFFSET, SEEK_SET);
	fread(height, 4, 1, streamIn);
	fseek(streamIn, WIDTH_OFFSET, SEEK_SET);
	fread(width, 4, 1, streamIn);

	//How many bits are each pixel?
	int bitsPerPixel = 0;
	fseek(streamIn, BITS_PER_PIXEL_OFFSET, SEEK_SET);
	fread(&bitsPerPixel, 2, 1, streamIn);
	*bytesPerPixel = bitsPerPixel / 8;

	//allocate enough memory to contain pixel data
	int paddedRowSize = (int)(4 * ceil((float)(*width) / 4.0f)) * (*bytesPerPixel);
	int unpaddedRowSize = *width * (*bytesPerPixel);
	int totalSize = unpaddedRowSize * (*height) * 3;

	*pixels = new char[totalSize];
	char temp[14400] = {};

	*colorTable = new char[256 * 4];

	//Take care of the Color Table. for 8-bit this will have 256 colors used
	//Color table is 4 x numColors Bytes
	fseek(streamIn, COLOR_TABLE_OFFSET, SEEK_SET);
	fread(*colorTable, 4, 256, streamIn);


	//Set address to be ready to read the last pixel first, then work backwords because bitmaps store data strangely
	char *currentRowPointer = *pixels + ((*height - 1) * unpaddedRowSize);

	for (int i = 0; i < *height; i++)
	{
		fseek(streamIn, dataOffset + (i * paddedRowSize), SEEK_SET);
		int test = fread(currentRowPointer, 1, unpaddedRowSize, streamIn);
		std::cout << "Size of read: " << test << "\n";
		currentRowPointer -= unpaddedRowSize;
	}

	fclose(streamIn);

	std::cin;
}

void cropBMP(const char* filename, char *pixels, int height, int width, int bytesPerPixel, char **colorTable){
	FILE *streamIn;
	errno_t err;
	//Read in image to crop
	err = fopen_s(&streamIn, filename, "rb");
	if (err != 0) {
		std::cout << "Error Reading file.";
	}

	int paddedRowSize = (int)(4 * ceil((float)(width) / 4.0f)) * bytesPerPixel;
	int unpaddedRowSize = width * bytesPerPixel;
	int totalSize = unpaddedRowSize * height * 3;

	//Loop through pixels, start logging which lines we don't want anymore.

}

void writeBMP(const char* filename, char *pixels, int height, int width, int bytesPerPixel, char **colorTable) {
	FILE *streamOut;
	errno_t err;
	//Read in image to crop
	err = fopen_s(&streamOut, "./GivenImg/fozzie-out.bmp", "wb");
	if (err != 0) {
		std::cout << "Error Reading file.";
	}
	//Write out header information
	//Bitmap File Header
	//BMP signature 2 bytes
	//const char *BM = "BM";
	fwrite("B", 1, 1, streamOut);
	fwrite("M", 1, 1, streamOut);
	//File size 4 bytes
	int paddedRowSize = (int)(4 * ceil((float)width / 4.0f))*bytesPerPixel;
	//paddedRowSize multiplied by height is our actual pixel data. Still need to add Bitmap header (14) and info header (40) and colorTable (1024) for full file size
	int fileSize = paddedRowSize * height + 1078;
	fwrite(&fileSize, 4, 1, streamOut);
	//Reserved 4 bytes
	int reserved = 0x0000;
	fwrite(&reserved, 4, 1, streamOut);
	//Data offset 4 bytes. Where the headers end and data starts. Bitmap head (14) + info header (40) + colorTable (1024) = 1078
	int dataOffset = 1078;
	fwrite(&dataOffset, 4, 1, streamOut);

	//Info header time
	//Size of the info header(40), 4 bytes
	int headerSize = 40;
	fwrite(&headerSize, 4, 1, streamOut);
	//Width, 4 bytes
	fwrite(&width, 4, 1, streamOut);
	//Height, 4 bytes
	fwrite(&height, 4, 1, streamOut);
	//Planes, 2 bytes
	int planes = 1;
	fwrite(&planes, 2, 1, streamOut);
	//Bits per pixel, 2 bytes
	int bitsPerPixel = bytesPerPixel * 8;
	fwrite(&bitsPerPixel, 2, 1, streamOut);
	//Compression (none), 4 bytes
	int comp = 0;
	fwrite(&comp, 4, 1, streamOut);
	//Image size, 4 bytes
	int imageSize = width * height*bytesPerPixel;
	fwrite(&imageSize, 4, 1, streamOut);
	//Pixels per meter in X axis and Y, 4 bytes
	int ppm = 11811;
	fwrite(&ppm, 4, 1, streamOut);
	fwrite(&ppm, 4, 1, streamOut);
	//Colors used, 4 bytes
	int colors = 256;
	fwrite(&colors, 4, 1, streamOut);
	//Important Colors, 4 bytes
	int impColors = 0;
	fwrite(&impColors, 4, 1, streamOut);
	//Color Table, 4 * number of colors
	fwrite(*colorTable, 4, 256, streamOut);

	//Pixel time. BMP is weird so start at the bottom and work our way up
	int unpaddedRowSize = width * bytesPerPixel;
	for (int i = 0; i < height; i++)
	{
		int pixelOffset = ((height - i) - 1) * unpaddedRowSize;
		fwrite(&pixels[pixelOffset], 1, paddedRowSize, streamOut);
	}
	fclose(streamOut);
}
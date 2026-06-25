#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef H_IMAGE_TGA
#define H_IMAGE_TGA

#pragma pack(push, 1)
typedef struct {
	char IDLength; // 
	char ColorMapType;
	char ImageType;

	// color map specification
	short ColorMapFirstIndex;
	short ColorMapLength;
	char 	ColorMapEntrySize;

	// image specification
	short XOrigin;
	short YOrigin;
	short Width;
	short Height;
	char BitsPerPixel;
	char Descriptor;
} TGAHEADER;

typedef struct {
	TGAHEADER Header;
	void *Data;
} TGAIMAGE;
#pragma pack(pop)

// RETURN VALUES: 
// -1 could not open the file 
// -2 the image encoding is not supported (yet)
int image_tga_ReadFile(TGAIMAGE *image, const char *filename);
#endif // H_IMAGE_TGA

#ifdef IMAGE_TGA_IMPLEMENTATION
// supported formats
#define FORMAT_RGB 3
#define FORMAT_RGBA 4
#define FORMAT_GRAYSCALE 1

int image_tga_ReadFile(TGAIMAGE *image, const char *filename)
{
	FILE* file; 
	int DataSize;

	// open the file in binary mode
	if (fopen_s(&file, filename, "rb") != 0){
		fprintf(stderr, "[FATAL]: error opening the file at: %s\n", filename);
		return -1;
	}

	// read the first 18 bytes into the header 
	assert(sizeof(image->Header) == fread(&(image->Header),sizeof(char),sizeof(image->Header),file));

	// check the resolution and the pixel depth
	int bytesPerPixel = image->Header.BitsPerPixel >> 3;
	assert((image->Header.Height > 0 && image->Header.Width > 0) && (bytesPerPixel == FORMAT_GRAYSCALE || bytesPerPixel == FORMAT_RGB || bytesPerPixel == FORMAT_RGBA));

	// allocate memory for the data
	DataSize = (image->Header.BitsPerPixel >> 3) * image->Header.Width * image->Header.Height;
	image->Data = (char *)malloc(DataSize);
	memset(image->Data, 0,DataSize); // initialize the memory to zero

	// read the data
	// printf("image->Header.ImageType: %d\n", image->Header.ImageType);
// 	printf("ftell(file): %d\n", ftell(file));

	switch(image->Header.ImageType) {
		case 0: {// no image data is provided in the file  
			fprintf(stderr, "[WARNING]: The image contains no actual data.\n");
			return (-3); 
		} break; 

		case 1: // uncompressed  color-mapped image
		{ 
			fprintf(stderr, "[FATAL]: Uncompressed color-mapped images are not supported yet :)\n"); 
			return (-2); 
		} break;

		case 2: // uncompressed true color image
		{ 
			// read the image data into image->Data field 
			// printf("ftell(file): %d\n", ftell(file));
		} break;

		case 3: // uncompressed GRAYSCALE image
		{ 
			fprintf(stderr, "[FATAL]: grayscale images are not supported yet :)\n"); 
			return (-2); 
		} break;

		case 9: // run-length encoded color-mapped image
		{ 
			fprintf(stderr, "[FATAL]: run-length encoded color-mapped images are not supported yet :)\n"); 
			return (-2);
		} break;

		case 10: // run-length encoded true-color image
		{ 
			fprintf(stderr, "[FATAL]: run-length encoded true-color images are not supported yet :)\n"); 
			return (-2);
		} break;

		case 11: // run-length encoded black and white(GRAYSCALE) image
		{ 
			fprintf(stderr, "[FATAL]: run-length encoded grayscale images are not supported yet :)\n"); 
			return (-2); 
		} break;

		default: {
			fprintf(stderr, "[FATAL]: Unsupprted image encoding\n");
			return (-2);
		} break;
	}

	fclose(file);
	return (0);
}

#endif//  IMAGE_TGA_IMPLEMENTATION

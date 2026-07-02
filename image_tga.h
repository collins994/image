#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef H_IMAGE_TGA
#define H_IMAGE_TGA

#pragma pack(push, 1)
typedef struct {
	unsigned char IDLength; // 
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

// TODO(collins): make the ImageID part of the ImageData
typedef struct {
	TGAHEADER Header;
	char *ImageID;
	void *ImageData;
} TGAIMAGE;
#pragma pack(pop)

// RETURN VALUES: 
// -1 could not open the file 
// -2 the image encoding is not supported (yet)
void image_tga_InitializeImage(TGAIMAGE *Image);
int image_tga_ReadFile(TGAIMAGE *image, const char *filename);
int image_tga_WriteFile(TGAIMAGE *image, const char *filename);
#endif // H_IMAGE_TGA

#ifdef IMAGE_TGA_IMPLEMENTATION
// supported formats
#define FORMAT_RGB 3
#define FORMAT_RGBA 4
#define FORMAT_GRAYSCALE 1
#define HEADER_SIZE 18

int image_tga_ReadFile(TGAIMAGE *image, const char *filename)
{
	FILE* file; 
	int DataSize;
	long fileSize;
	int bytesPerPixel;

	// open the file in binary mode
	if (fopen_s(&file, filename, "rb") != 0){
		fprintf(stderr, "[FATAL]: error opening the file at: %s\n", filename);
		return -1;
	}

	// get the file size
	fseek(file, 0, SEEK_END);
	fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	// write the header
	// -> make sure the file is large enough
	if (fileSize < 18) {
		fprintf(stderr, "[FATAL]: (%s) File too small\n", filename);
		return -1;
	}
	if((fread(&(image->Header), 1, 18, file)) < 18){
		fprintf(stderr, "[FATAL]: (%s) Can't dump the file\n", filename);
		fprintf(stderr, "[DEBUG]: %s(%d)", __FILE__, __LINE__);
		return -1;
	}

	// write the image data
	// -> make sure the image actually contains any data (as indicated by the ImageType in the Header)
	if (image->Header.ImageType == 0) 
	{
		fprintf(stderr, "[WARNING]: (%s) Contains no actual data.", filename);
		return -2;
	}
	// -> make sure the image bit depth is one of FORMAT_RGB, FORMAT_RGBA or FORMAT_GRAYSCALE
	bytesPerPixel = (image->Header.BitsPerPixel >> 3);
	if (!(bytesPerPixel == FORMAT_GRAYSCALE || bytesPerPixel == FORMAT_RGB || bytesPerPixel == FORMAT_RGBA)) 
	{
		fprintf(stderr, "[FATAL]: (%s) Unsupported number of bytes per pixel.\n", filename);
		return -1;
	}  
	// -> make sure the image file is large enough (as indicated by the resolution and IDLength);
	DataSize = (image->Header.IDLength + (image->Header.Width * image->Header.Height * bytesPerPixel));
	if(fileSize < (DataSize))
	{
		fprintf(stderr, "[FATAL]: (%s) File too small\n", filename);
		return -1;
	}

	// -> allocate space 
	image->ImageData = (char *)malloc(DataSize);
	memset(image->ImageData, 0,DataSize); // initialize the memory to zeros

	switch(image->Header.ImageType) {
		case 1: // uncompressed  color-mapped image
		{ 
			fprintf(stderr, "[FATAL]: Uncompressed color-mapped images are not supported yet :)\n"); 
			return (-1); 
		} break;

		case 2: // uncompressed true color image
		{ 
			// read the image data into image->ImageData field 
			assert(DataSize == fread((image->ImageData), 1, DataSize, file));
		} break;

		case 3: // uncompressed GRAYSCALE image
		{ 
			fprintf(stderr, "[FATAL]: grayscale images are not supported yet :)\n"); 
			return (-1); 
		} break;

		case 9: // run-length encoded color-mapped image
		{ 
			fprintf(stderr, "[FATAL]: run-length encoded color-mapped images are not supported yet :)\n"); 
			return (-1);
		} break;

		case 10: // run-length encoded true-color image
		{ 
			fprintf(stderr, "[FATAL]: run-length encoded true-color images are not supported yet :)\n"); 
			return (-1);
		} break;

		case 11: // run-length encoded black and white(GRAYSCALE) image
		{ 
			fprintf(stderr, "[FATAL]: run-length encoded grayscale images are not supported yet :)\n"); 
			return (-1); 
		} break;

		default: {
			fprintf(stderr, "[FATAL]: unsupported image encoding\n");
			return (-2);
		} break;
	}

	fclose(file);
	return (0);
}

void ReleaseImage(TGAIMAGE *image) 
{
	free(image->ImageData);
}

int image_tga_WriteFile(TGAIMAGE *Image, const char *filename)
{
	FILE *file;
	long DataSize;

	// open the file
	if (fopen_s(&file, filename, "wb") != 0){
		fprintf(stderr, "[FATAL]: error opening the file at: %s\n", filename);
		return -1;
	}

	// check if the user gave an image id
	if(Image->ImageID != NULL) { Image->Header.IDLength = strlen(Image->ImageID); }

	// write the image header
	if((fwrite(&(Image->Header), 1, HEADER_SIZE, file)) < HEADER_SIZE) // TODO: find a way to handle this error properly (inform the user what to do)
	{
	 	fprintf(stderr, "[FATAL]: (%s) could not write file\n", filename);
	 	fprintf(stderr, "[DEBUG]: file: %s, line: %d\n", __FILE__, __LINE__);
	 	return -1;
	}

	// write the image
	// -> make sure that the data in Image is correct according to the image header
	DataSize = (Image->Header.BitsPerPixel >> 3) * (Image->Header.Width * Image->Header.Height) + Image->Header.IDLength;  // (bytesPerPixel * numberofpixels) + numberofbytesforid
	if (fwrite((Image->ImageData), 1, DataSize, file) < DataSize)
	{
	 	fprintf(stderr, "[FATAL]: (%s) could not write file\n", filename);
	 	fprintf(stderr, "[DEBUG]: file: %s, line: %d\n", __FILE__, __LINE__);
	 	return -1;
	}

	fclose(file);
	return 0;
}

void image_tga_InitializeImage(TGAIMAGE *Image)
{
	Image->ImageID = NULL;
}
#endif//  IMAGE_TGA_IMPLEMENTATION

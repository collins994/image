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
int image_tga_WriteFile(TGAIMAGE *image, const char *filename);
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

	// make sure the image is atleast 18 bytes large
	fseek(file, 0, SEEK_END);
	if(ftell(file) < 18){
		fprintf(stderr, "[FATAL]: (%s) The file header is too small\n", filename);
		return -1;
	}
	fseek(file, 0, SEEK_SET);
	// read the first 18 bytes into the header 
	// assert(sizeof(image->Header) == fread(&(image->Header),sizeof(char),sizeof(image->Header),file));
	if(18 < fread(&(image->Header), sizeof(char), 18, file)){
		fprintf(stderr, "[FATAL]: The header of %s is less than 18 bytes\n", filename);
		return -1;
	}
	
	// check if the image has an ID section
	if(image->Header.IDLength > 0) {
		printf("[INFO]: Image id length: %d\n", image->Header.IDLength);
	}

	// check the resolution and the pixel depth
	int bytesPerPixel = image->Header.BitsPerPixel >> 3;
	// assert((image->Header.Height > 0 && image->Header.Width > 0) && (bytesPerPixel == FORMAT_GRAYSCALE || bytesPerPixel == FORMAT_RGB || bytesPerPixel == FORMAT_RGBA));
 if(!((image->Header.Height > 0 && image->Header.Width > 0) && (bytesPerPixel == FORMAT_GRAYSCALE || bytesPerPixel == FORMAT_RGB || bytesPerPixel == FORMAT_RGBA))){
 		fprintf(stderr, "[FATAL]: the file format for %s is not recognized\n", filename);
		return -1;
 }

	if(image->Header.ImageType == 0) {
			fprintf(stderr, "[WARNING]: The image contains no actual data.\n");
			return (-1); 
	}

	// allocate memory for the data
	DataSize = (image->Header.BitsPerPixel >> 3) * image->Header.Width * image->Header.Height;
	image->Data = (char *)malloc(DataSize);
	memset(image->Data, 0,DataSize); // initialize the memory to zero
	
	// make sure the file is large enough before reading the data
	fseek(file, 0, SEEK_END);
	if((ftell(file) - 18) < (DataSize)){
		fprintf(stderr, "[FATAL]: (%s) The file data field is too small\n", filename);
		return -1;
	}
	fseek(file, 0, SEEK_SET);

	// read the data
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
			assert(DataSize == fread((image->Data), 1, DataSize, file));
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

void ReleaseImage(TGAIMAGE *image) 
{
	free(image->Data);
}

int image_tga_WriteFile(TGAIMAGE *image, const char *filename)
{
	FILE *file;

	// open the file
	if (fopen_s(&file, filename, "wb") != 0){
		fprintf(stderr, "[FATAL]: error opening the file at: %s\n", filename);
		return -1;
	}

	// write the header
	// TODO(collins): handle this error properly
	if(fwrite(&(image->Header), 1, 18, file) < 18)
	{
		fprintf(stderr, "[FATAL]: could not dumb the tga file\n");
		return -1;
	}

	fclose(file);
	return 0;
}

#endif//  IMAGE_TGA_IMPLEMENTATION

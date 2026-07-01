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

	// open the file in binary mode
	if (fopen_s(&file, filename, "rb") != 0){
		fprintf(stderr, "[FATAL]: error opening the file at: %s\n", filename);
		return -1;
	}

	// make sure the image is atleast 18 bytes large
	fseek(file, 0, SEEK_END);
	if(ftell(file) < HEADER_SIZE){
		fprintf(stderr, "[FATAL]: (%s) The file header is too small\n", filename);
		return -1;
	}
	fseek(file, 0, SEEK_SET);
	// TODO(collins): handle this error properly
	if(HEADER_SIZE < fread(&(image->Header), sizeof(char), HEADER_SIZE, file)){
		fprintf(stderr, "[FATAL]: The header of %s is less than 18 bytes\n", filename);
		return -1;
	}
	
	// check if the image has an ID section,
	// write the bytes into the Image->ImageID (not into the ImageData)
	if(image->Header.IDLength > 0) {
		image->ImageID = (char *)malloc(sizeof(char));
		fread(image->ImageID, 1, (image->Header).IDLength, file);
		printf("[INFO]: image id (%.*s)\n", (image->Header.IDLength), image->ImageID);
	}

	// check the resolution and the pixel depth
	int bytesPerPixel = image->Header.BitsPerPixel >> 3;
	if(!((image->Header.Height > 0 && image->Header.Width > 0) && (bytesPerPixel == FORMAT_GRAYSCALE || bytesPerPixel == FORMAT_RGB || bytesPerPixel == FORMAT_RGBA))){
		fprintf(stderr, "[FATAL]: the file format for %s is not recognized\n", filename);
		return -1;
	}

	// make sure the header indicates that the file has actual data
	DataSize = (image->Header.BitsPerPixel >> 3) * image->Header.Width * image->Header.Height;
	if(image->Header.ImageType == 0) {
			fprintf(stderr, "[WARNING]: The image contains no actual data.\n");
			return (-1); 
	}
	// make sure the file is large enough before reading the data
	fseek(file, 0, SEEK_END);
	if((ftell(file) - HEADER_SIZE) < (DataSize)){
		fprintf(stderr, "[FATAL]: (%s) The data field is too small.\n", filename);
		return -1;
	}
	fseek(file, 0, SEEK_SET);

	// allocate memory for the data
	image->ImageData = (char *)malloc(DataSize);
	memset(image->ImageData, 0,DataSize); // initialize the memory to zero

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
			// read the image data into image->ImageData field 
			assert(DataSize == fread((image->ImageData), 1, DataSize, file));
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
	free(image->ImageData);
}

int image_tga_WriteFile(TGAIMAGE *Image, const char *filename)
{
	FILE *file;

	// open the file
	if (fopen_s(&file, filename, "wb") != 0){
		fprintf(stderr, "[FATAL]: error opening the file at: %s\n", filename);
		return -1;
	}

	/**===== Writing the header ==== */
	// check if the user gave an image id
	if(Image->ImageID != NULL) { Image->Header.IDLength = strlen(Image->ImageID); }
	// write the header
	// TODO(collins): handle this error properly
	if(fwrite(&(Image->Header), 1, HEADER_SIZE, file) < HEADER_SIZE)
	{
		fprintf(stderr, "[FATAL]: could not write the tga file\n");
		fprintf(stderr, "[DEBUG]: file: %s, line: %d\n", __FILE__, __LINE__);
		return -1;
	}


	// write the image id before writing the image data
	// if((fwrite((Image + HEADER_SIZE), 1, Image->Header.IDLength, file)) < Image->Header.IDLength){
	if((fwrite((Image->ImageID), 1, Image->Header.IDLength, file)) < Image->Header.IDLength){
		fprintf(stderr, "[FATAL]: could not write the tga file\n");
		fprintf(stderr, "[DEBUG]: file: %s, line: %d\n", __FILE__, __LINE__);
		return -1;
	}

	fclose(file);
	return 0;
}

#endif//  IMAGE_TGA_IMPLEMENTATION

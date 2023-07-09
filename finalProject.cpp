#include <iostream>
//#include <GL/gl.h>
//#include <GL/glu.h>
#include <glut.h>
#include <Windows.h>
#include "RgbImage.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
/*
Rodar o manípulo - W A S D
Botão Azul - B
Botão Verde - N
Zoom In - P
Zoom Out - O
Aumentar transparencia - L
Diminuir transparencia - K
On/Off Light 0-G
On/Off Light 1-H
Increase angle - I
Decrease angle - U
Rotate Z - C -- V
Rotate X - Z -- X 
*/
int test = 60;
int flag = 0;

RgbImage imag;
void display();
void reshape(int w, int h);
void init();
void timer(int);
int angle=0;
int angleCamY = 0;
float vel = 1;
float transG = 0;
float transB = 0;
float angleX = 0;
float angleY = 0;
float angleF = 25;





GLuint   texture[4];
GLfloat intensity = 0.2f;
GLfloat luzGlobalCorAmb[4] = { intensity, intensity,intensity, 1.0 };
GLfloat   luzR = 1;		 	 //:::   'R'  
GLfloat   luzG = 1;			 //:::   'G'  
GLfloat   luzB = 1;			 //:::   'B' 
GLfloat   posX = 0.0;
GLfloat   posY = 0.0;
GLfloat transpCoef = 1.0;
GLfloat localCorAmb[4] = { 0, 0, 0, 0.0 };
GLfloat localCorDif[4] = { luzR, luzG, luzB, 1.0 };
GLfloat localCorEsp[4] = { luzR, luzG, luzB, 1.0 };
GLfloat whitePlasticAmb[] = { 0.8 ,0.8 ,0.8 };
GLfloat whitePlasticDif[] = { 0.55 ,0.55 ,0.55 };
GLfloat whitePlasticSpec[] = { 0.870 ,0.870 ,0.870 };
GLint whitePlasticCoef = 128;
int light0State = 1;
int light1State = 1;
GLfloat   FluzR = 1;		 	 //:::   'R'  
GLfloat   FluzG = 1;			 //:::   'G'  
GLfloat   FluzB = 1;			 //:::   'B' 



RgbImage::RgbImage(int numRows, int numCols)
{
	NumRows = numRows;
	NumCols = numCols;
	ImagePtr = new unsigned char[NumRows * GetNumBytesPerRow()];
	if (!ImagePtr) {
		fprintf(stderr, "Unable to allocate memory for %ld x %ld bitmap.\n",
			NumRows, NumCols);
		Reset();
		ErrorCode = MemoryError;
	}
	// Zero out the image
	unsigned char* c = ImagePtr;
	int rowLen = GetNumBytesPerRow();
	for (int i = 0; i < NumRows; i++) {
		for (int j = 0; j < rowLen; j++) {
			*(c++) = 0;
		}
	}
}
/* ********************************************************************
 *  LoadBmpFile
 *  Read into memory an RGB image from an uncompressed BMP file.
 *  Return true for success, false for failure.  Error code is available
 *     with a separate call.
 *  Author: Sam Buss December 2001.
 **********************************************************************/
bool RgbImage::LoadBmpFile(const char* filename)
{

	Reset();

	FILE* infile;
	if (fopen_s(&infile, filename, "rb")) {
		fprintf(stderr, "Unable to open file: %s\n", filename);
		ErrorCode = OpenError;
		return false;
	}

	bool fileFormatOK = false;
	int bChar = fgetc(infile);
	int mChar = fgetc(infile);
	if (bChar == 'B' && mChar == 'M') {			// If starts with "BM" for "BitMap"
		skipChars(infile, 4 + 2 + 2 + 4 + 4);			// Skip 4 fields we don't care about
		NumCols = readLong(infile);
		NumRows = readLong(infile);
		skipChars(infile, 2);					// Skip one field
		int bitsPerPixel = readShort(infile);
		skipChars(infile, 4 + 4 + 4 + 4 + 4 + 4);		// Skip 6 more fields

		if (NumCols > 0 && NumCols <= 100000 && NumRows > 0 && NumRows <= 100000
			&& bitsPerPixel == 24 && !feof(infile)) {
			fileFormatOK = true;
		}
	}
	if (!fileFormatOK) {
		Reset();
		ErrorCode = FileFormatError;
		fprintf(stderr, "Not a valid 24-bit bitmap file: %s.\n", filename);
		fclose(infile);
		return false;
	}

	// Allocate memory
	ImagePtr = new unsigned char[NumRows * GetNumBytesPerRow()];
	if (!ImagePtr) {
		fprintf(stderr, "Unable to allocate memory for %ld x %ld bitmap: %s.\n",
			NumRows, NumCols, filename);
		Reset();
		ErrorCode = MemoryError;
		fclose(infile);
		return false;
	}

	unsigned char* cPtr = ImagePtr;
	for (int i = 0; i < NumRows; i++) {
		int j;
		for (j = 0; j < NumCols; j++) {
			*(cPtr + 2) = fgetc(infile);	// Blue color value
			*(cPtr + 1) = fgetc(infile);	// Green color value
			*cPtr = fgetc(infile);		// Red color value
			cPtr += 3;
		}
		int k = 3 * NumCols;					// Num bytes already read
		for (; k < GetNumBytesPerRow(); k++) {
			fgetc(infile);				// Read and ignore padding;
			*(cPtr++) = 0;
		}
	}
	if (feof(infile)) {
		fprintf(stderr, "Premature end of file: %s.\n", filename);
		Reset();
		ErrorCode = ReadError;
		fclose(infile);
		return false;
	}
	fclose(infile);	// Close the file
	return true;
}
short RgbImage::readShort(FILE* infile)
{
	// read a 16 bit integer
	unsigned char lowByte, hiByte;
	lowByte = fgetc(infile);			// Read the low order byte (little endian form)
	hiByte = fgetc(infile);			// Read the high order byte

	// Pack together
	short ret = hiByte;
	ret <<= 8;
	ret |= lowByte;
	return ret;
}
long RgbImage::readLong(FILE* infile)
{
	// Read in 32 bit integer
	unsigned char byte0, byte1, byte2, byte3;
	byte0 = fgetc(infile);			// Read bytes, low order to high order
	byte1 = fgetc(infile);
	byte2 = fgetc(infile);
	byte3 = fgetc(infile);

	// Pack together
	long ret = byte3;
	ret <<= 8;
	ret |= byte2;
	ret <<= 8;
	ret |= byte1;
	ret <<= 8;
	ret |= byte0;
	return ret;
}
void RgbImage::skipChars(FILE* infile, int numChars)
{
	for (int i = 0; i < numChars; i++) {
		fgetc(infile);
	}
}
/* ********************************************************************
 *  WriteBmpFile
 *  Write an RGB image to an uncompressed BMP file.
 *  Return true for success, false for failure.  Error code is available
 *     with a separate call.
 *  Author: Sam Buss, January 2003.
 **********************************************************************/
bool RgbImage::WriteBmpFile(const char* filename)
{

	//errno_t err;
	FILE* outfile;


	if (fopen_s(&outfile, filename, "wb")) {
		fprintf(stderr, "Unable to open file: %s\n", filename);
		ErrorCode = OpenError;
		return false;
	}

	fputc('B', outfile);
	fputc('M', outfile);
	int rowLen = GetNumBytesPerRow();
	writeLong(40 + 14 + NumRows * rowLen, outfile);	// Length of file
	writeShort(0, outfile);					// Reserved for future use
	writeShort(0, outfile);
	writeLong(40 + 14, outfile);				// Offset to pixel data
	writeLong(40, outfile);					// header length
	writeLong(NumCols, outfile);				// width in pixels
	writeLong(NumRows, outfile);				// height in pixels (pos for bottom up)
	writeShort(1, outfile);		// number of planes
	writeShort(24, outfile);		// bits per pixel
	writeLong(0, outfile);		// no compression
	writeLong(0, outfile);		// not used if no compression
	writeLong(0, outfile);		// Pixels per meter
	writeLong(0, outfile);		// Pixels per meter
	writeLong(0, outfile);		// unused for 24 bits/pixel
	writeLong(0, outfile);		// unused for 24 bits/pixel

	// Now write out the pixel data:
	unsigned char* cPtr = ImagePtr;
	for (int i = 0; i < NumRows; i++) {
		// Write out i-th row's data
		int j;
		for (j = 0; j < NumCols; j++) {
			fputc(*(cPtr + 2), outfile);		// Blue color value
			fputc(*(cPtr + 1), outfile);		// Blue color value
			fputc(*(cPtr + 0), outfile);		// Blue color value
			cPtr += 3;
		}
		// Pad row to word boundary
		int k = 3 * NumCols;					// Num bytes already read
		for (; k < GetNumBytesPerRow(); k++) {
			fputc(0, outfile);				// Read and ignore padding;
			cPtr++;
		}
	}

	fclose(outfile);	// Close the file
	return true;
}
void RgbImage::writeLong(long data, FILE* outfile)
{
	// Read in 32 bit integer
	unsigned char byte0, byte1, byte2, byte3;
	byte0 = (unsigned char)(data & 0x000000ff);		// Write bytes, low order to high order
	byte1 = (unsigned char)((data >> 8) & 0x000000ff);
	byte2 = (unsigned char)((data >> 16) & 0x000000ff);
	byte3 = (unsigned char)((data >> 24) & 0x000000ff);

	fputc(byte0, outfile);
	fputc(byte1, outfile);
	fputc(byte2, outfile);
	fputc(byte3, outfile);
}
void RgbImage::writeShort(short data, FILE* outfile)
{
	// Read in 32 bit integer
	unsigned char byte0, byte1;
	byte0 = data & 0x000000ff;		// Write bytes, low order to high order
	byte1 = (data >> 8) & 0x000000ff;

	fputc(byte0, outfile);
	fputc(byte1, outfile);
}
/*********************************************************************
 * SetRgbPixel routines allow changing the contents of the RgbImage. *
 *********************************************************************/
void RgbImage::SetRgbPixelf(long row, long col, double red, double green, double blue)
{
	SetRgbPixelc(row, col, doubleToUnsignedChar(red),
		doubleToUnsignedChar(green),
		doubleToUnsignedChar(blue));
}
void RgbImage::SetRgbPixelc(long row, long col,
	unsigned char red, unsigned char green, unsigned char blue)
{
	assert(row < NumRows&& col < NumCols);
	unsigned char* thePixel = GetRgbPixel(row, col);
	*(thePixel++) = red;
	*(thePixel++) = green;
	*(thePixel) = blue;
}
unsigned char RgbImage::doubleToUnsignedChar(double x)
{
	if (x >= 1.0) {
		return (unsigned char)255;
	}
	else if (x <= 0.0) {
		return (unsigned char)0;
	}
	else {
		return (unsigned char)(x * 255.0);		// Rounds down
	}
}
// Bitmap file format  (24 bit/pixel form)		BITMAPFILEHEADER
// Header (14 bytes)
//	 2 bytes: "BM"
//   4 bytes: long int, file size
//   4 bytes: reserved (actually 2 bytes twice)
//   4 bytes: long int, offset to raster data
// Info header (40 bytes)						BITMAPINFOHEADER
//   4 bytes: long int, size of info header (=40)
//	 4 bytes: long int, bitmap width in pixels
//   4 bytes: long int, bitmap height in pixels
//   2 bytes: short int, number of planes (=1)
//   2 bytes: short int, bits per pixel
//   4 bytes: long int, type of compression (not applicable to 24 bits/pixel)
//   4 bytes: long int, image size (not used unless compression is used)
//   4 bytes: long int, x pixels per meter
//   4 bytes: long int, y pixels per meter
//   4 bytes: colors used (not applicable to 24 bit color)
//   4 bytes: colors important (not applicable to 24 bit color)
// "long int" really means "unsigned long int"
// Pixel data: 3 bytes per pixel: RGB values (in reverse order).
//	Rows padded to multiples of four.
#ifndef RGBIMAGE_DONT_USE_OPENGL
bool RgbImage::LoadFromOpenglBuffer()					// Load the bitmap from the current OpenGL buffer
{
	int viewportData[4];
	glGetIntegerv(GL_VIEWPORT, viewportData);
	int& vWidth = viewportData[2];
	int& vHeight = viewportData[3];

	if (ImagePtr == 0) { // If no memory allocated
		NumRows = vHeight;
		NumCols = vWidth;
		ImagePtr = new unsigned char[NumRows * GetNumBytesPerRow()];
		if (!ImagePtr) {
			fprintf(stderr, "Unable to allocate memory for %ld x %ld buffer.\n",
				NumRows, NumCols);
			Reset();
			ErrorCode = MemoryError;
			return false;
		}
	}
	assert(vWidth >= NumCols && vHeight >= NumRows);
	int oldGlRowLen;
	if (vWidth >= NumCols) {
		glGetIntegerv(GL_UNPACK_ROW_LENGTH, &oldGlRowLen);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, NumCols);
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	// Get the frame buffer data.
	glReadPixels(0, 0, NumCols, NumRows, GL_RGB, GL_UNSIGNED_BYTE, ImagePtr);

	// Restore the row length in glPixelStorei  (really ought to restore alignment too).
	if (vWidth >= NumCols) {
		glPixelStorei(GL_UNPACK_ROW_LENGTH, oldGlRowLen);
	}
	return true;
}

#endif   // RGBIMAGE_DONT_USE_OPENGL

void initTexturas()
{

	glGenTextures(1, &texture[0]);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	imag.LoadBmpFile("marmore.bmp");
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, 3,
		imag.GetNumCols(),
		imag.GetNumRows(), 0, GL_RGB, GL_UNSIGNED_BYTE,
		imag.ImageData());



	glGenTextures(1, &texture[1]);
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	imag.LoadBmpFile("wood.bmp");
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, 3,
		imag.GetNumCols(),
		imag.GetNumRows(), 0, GL_RGB, GL_UNSIGNED_BYTE,
		imag.ImageData());


	glGenTextures(1, &texture[2]);
	glBindTexture(GL_TEXTURE_2D, texture[2]);
	imag.LoadBmpFile("wall.bmp");
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, 3,
		imag.GetNumCols(),
		imag.GetNumRows(), 0, GL_RGB, GL_UNSIGNED_BYTE,
		imag.ImageData());

	glGenTextures(1, &texture[3]);
	glBindTexture(GL_TEXTURE_2D, texture[3]);
	imag.LoadBmpFile("sistina.bmp");
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, 3,
		imag.GetNumCols(),
		imag.GetNumRows(), 0, GL_RGB, GL_UNSIGNED_BYTE,
		imag.ImageData());
}
void initLights() {
	GLfloat localPos[4] = { posX,posY,12.0 , 1.0 };
	glLightfv(GL_LIGHT0, GL_POSITION, localPos);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, localCorDif);
	GLfloat PosFOCO[4] = { 0,0,12 , 1.0 };
	GLfloat corFOCO[4] = { FluzR, FluzG, FluzB, 1.0 };
	GLfloat cDIRECTION[4] = { 0, 0, -1, 1.0 };
	
	glLightfv(GL_LIGHT1, GL_POSITION, PosFOCO);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, corFOCO);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, cDIRECTION);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, angleF);
	
}
void initMaterials() {
	//glMaterialfv(GL_FRONT, GL_AMBIENT, whitePlasticAmb);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, whitePlasticDif);
	glMaterialf(GL_FRONT, GL_SHININESS, whitePlasticCoef);
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(200, 100);
	glutInitWindowSize(500, 500);

	glutCreateWindow("Window");
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutTimerFunc(1000, timer, 0);


	glutMainLoop();
	return 0;
}

void drawCubeMATERIAL(float x, float y, float z, float larg, float alt, float expr) {
	//initMaterials();
	glNormal3f(0, 0, 1);
	glBegin(GL_QUADS);
	glVertex3f(x + larg / 2, y + alt / 2, z + expr / 2);
	glVertex3f(x - larg / 2, y + alt / 2, z + expr / 2);
	glVertex3f(x - larg / 2, y - alt / 2, z + expr / 2);
	glVertex3f(x + larg / 2, y - alt / 2, z + expr / 2);
	glEnd();

	// Back face 
	//initMaterials();
	glNormal3f(0, 0, -1);
	glBegin(GL_QUADS);
	glVertex3f(x + larg / 2, y + alt / 2, z - expr / 2);
	glVertex3f(x - larg / 2, y + alt / 2, z - expr / 2);
	glVertex3f(x - larg / 2, y - alt / 2, z - expr / 2);
	glVertex3f(x + larg / 2, y - alt / 2, z - expr / 2);
	glEnd();

	//Right face
	//initMaterials();
	glNormal3f(1, 0, 0);
	glBegin(GL_QUADS);
	glVertex3f(x + larg / 2, y + alt / 2, z - expr / 2);
	glVertex3f(x + larg / 2, y + alt / 2, z + expr / 2);
	glVertex3f(x + larg / 2, y - alt / 2, z + expr / 2);
	glVertex3f(x + larg / 2, y - alt / 2, z - expr / 2);
	glEnd();
	// Left face
	//initMaterials();
	glNormal3f(-1, 0, 0);
	glBegin(GL_QUADS);
	glVertex3f(x - larg / 2, y + alt / 2, z - expr / 2);
	glVertex3f(x - larg / 2, y + alt / 2, z + expr / 2);
	glVertex3f(x - larg / 2, y - alt / 2, z + expr / 2);
	glVertex3f(x - larg / 2, y - alt / 2, z - expr / 2);
	glEnd();
	// Bottom face
	//initMaterials();
	glNormal3f(0, -1, 0);
	glBegin(GL_QUADS);
	glVertex3f(x - larg / 2, y - alt / 2, z - expr / 2);
	glVertex3f(x - larg / 2, y - alt / 2, z + expr / 2);
	glVertex3f(x + larg / 2, y - alt / 2, z + expr / 2);
	glVertex3f(x + larg / 2, y - alt / 2, z - expr / 2);
	glEnd();
	// Top face
	//initMaterials();
	glNormal3f(0, 1, 0);
	glBegin(GL_QUADS);
	glVertex3f(x - larg / 2, y + alt / 2, z - expr / 2);
	glVertex3f(x - larg / 2, y + alt / 2, z + expr / 2);
	glVertex3f(x + larg / 2, y + alt / 2, z + expr / 2);
	glVertex3f(x + larg / 2, y + alt / 2, z - expr / 2);


	glEnd();

}
void drawCubeTEXT(float x, float y, float z, float larg, float alt, float expr, GLuint text) {
	GLfloat branco[4] = { 1, 1, 1, 1.0 };
	
	glEnable(GL_TEXTURE_2D);
	//FRONTE FACE
	int dim = 64;
	float med_dim = dim / 2;
	
	glBindTexture(GL_TEXTURE_2D, text);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, branco);
	glMaterialf(GL_FRONT, GL_SHININESS, 128);
	glNormal3f(0, 0, 1);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);    glVertex3i(x + larg / 2, y + alt / 2, z + expr / 2); //A 
	glTexCoord2f(1.0f, 0.0f); 	 glVertex3i(x - larg / 2, y + alt / 2, z + expr / 2);  //B
	glTexCoord2f(1.0f, 1.0f);    glVertex3i(x - larg / 2, y - alt / 2, z + expr / 2); //C
	glTexCoord2f(0.0f, 1.0f);  	 glVertex3i(x + larg / 2, y - alt / 2, z + expr / 2); //D
	glEnd();
	/*glBegin(GL_QUADS);
	for (int i = 0; i < dim; i++)
		for (int j = 0; j < dim; j++) {
			
			glTexCoord2f((float)j / dim, (float)i / dim);
			glVertex3d((float)j / med_dim, (float)i / med_dim, z + expr / 2);

			glTexCoord2f((float)(j + 1) / dim, (float)i / dim);
			glVertex3d((float)(j + 1) / med_dim, (float)i / med_dim, z + expr / 2);

			glTexCoord2f((float)(j + 1) / dim, (float)(i + 1) / dim);
			glVertex3d((float)(j + 1) / med_dim, (float)(i + 1) / med_dim, z + expr / 2);

			glTexCoord2f((float)j / dim, (float)(i + 1) / dim);
			glVertex3d((float)j / med_dim, (float)(i + 1) / med_dim, z + expr / 2);
			
		}
	glEnd();*/
	//BACK FACE
	glBindTexture(GL_TEXTURE_2D, text);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);    glVertex3i(x + larg / 2, y + alt / 2, z - expr / 2); //A 
	glTexCoord2f(1.0f, 0.0f); 	 glVertex3i(x - larg / 2, y + alt / 2, z - expr / 2);  //B
	glTexCoord2f(1.0f, 1.0f);    glVertex3i(x - larg / 2, y - alt / 2, z - expr / 2); //C
	glTexCoord2f(0.0f, 1.0f);  	 glVertex3i(x + larg / 2, y - alt / 2, z - expr / 2); //D
	glEnd();
	//Right FACE
	glBindTexture(GL_TEXTURE_2D, text);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);    glVertex3i(x + larg / 2, y + alt / 2, z - expr / 2); //A 
	glTexCoord2f(1.0f, 0.0f); 	 glVertex3i(x + larg / 2, y + alt / 2, z + expr / 2);  //B
	glTexCoord2f(1.0f, 1.0f);    glVertex3i(x + larg / 2, y - alt / 2, z + expr / 2); //C
	glTexCoord2f(0.0f, 1.0f);  	 glVertex3i(x + larg / 2, y - alt / 2, z - expr / 2); //D
	glEnd();
	//LefT FACE
	glBindTexture(GL_TEXTURE_2D, text);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);    glVertex3i(x - larg / 2, y + alt / 2, z - expr / 2); //A 
	glTexCoord2f(1.0f, 0.0f); 	 glVertex3i(x - larg / 2, y + alt / 2, z + expr / 2);  //B
	glTexCoord2f(1.0f, 1.0f);    glVertex3i(x - larg / 2, y - alt / 2, z + expr / 2); //C
	glTexCoord2f(0.0f, 1.0f);  	 glVertex3i(x - larg / 2, y - alt / 2, z - expr / 2); //D
	glEnd();
	//BOTTOM FACE
	glBindTexture(GL_TEXTURE_2D, text);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);    glVertex3i(x - larg / 2, y - alt / 2, z - expr / 2); //A 
	glTexCoord2f(1.0f, 0.0f); 	 glVertex3i(x - larg / 2, y - alt / 2, z + expr / 2);  //B
	glTexCoord2f(1.0f, 1.0f);    glVertex3i(x + larg / 2, y - alt / 2, z + expr / 2); //C
	glTexCoord2f(0.0f, 1.0f);  	 glVertex3i(x + larg / 2, y - alt / 2, z - expr / 2); //D
	glEnd();
	//TOP FACE
	glBindTexture(GL_TEXTURE_2D, text);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);    glVertex3i(x - larg / 2, y + alt / 2, z - expr / 2); //A 
	glTexCoord2f(1.0f, 0.0f); 	 glVertex3i(x - larg / 2, y + alt / 2, z + expr / 2);  //B
	glTexCoord2f(1.0f, 1.0f);    glVertex3i(x + larg / 2, y + alt / 2, z + expr / 2); //C
	glTexCoord2f(0.0f, 1.0f);  	 glVertex3i(x + larg / 2, y + alt / 2, z - expr / 2); //D
	glEnd();

	glDisable(GL_TEXTURE_2D);


}
void drawSquare(float x, float y, float z, float larg, float alt, GLuint text) {
	GLfloat branco[4] = { 1, 1, 1, 1.0 };
	float xAux = 1 / 25;
	float yAux = 1 / 14;
	float nrQuadX = x + 12;
	float nrQuadY = y + 7;
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, text);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, branco);
	glMaterialf(GL_FRONT, GL_SHININESS, 128);
	glNormal3f(0, 0, 1);
	glBegin(GL_QUADS);
	
	glTexCoord2f(0.0, 0.0);    glVertex3i(x + larg / 2, y + alt / 2, z); //A 
	glTexCoord2f(1.0, 0.0); 	 glVertex3i(x - larg / 2, y + alt / 2, z);  //B
	glTexCoord2f(1.0, 1.0);    glVertex3i(x - larg / 2, y - alt / 2, z); //C
	glTexCoord2f(0.0, 1.0);  	 glVertex3i(x + larg / 2, y - alt / 2, z); //D
	
	
	/*glTexCoord2f((nrQuadX * xAux), (nrQuadY * yAux));          glVertex3i(x + larg / 2, y + alt / 2, z); //A 
	glTexCoord2f((nrQuadX * xAux) + xAux, (nrQuadY * yAux)); 	       glVertex3i(x - larg / 2, y + alt / 2, z);  //B
	glTexCoord2f((nrQuadX * xAux) + xAux, (nrQuadY * yAux)+yAux);       glVertex3i(x - larg / 2, y - alt / 2, z); //C
	glTexCoord2f((nrQuadX * xAux), (nrQuadY * yAux) + yAux);  	     glVertex3i(x + larg / 2, y - alt / 2, z); //D*/
	
	glEnd();
	glDisable(GL_TEXTURE_2D);
}
void drawMalha() {
	int dimX = 12;
	int dimY = 7;
	

	for (int i = -12; i <= dimX; i++)
		for (int j = -7; j <= dimY; j++) {

			drawSquare(i, j, -0.5, 1, 1, texture[0]);

		}


}
void drawTampo(float x, float y, float z, float larg, float alt, float expr, GLuint text) {
	if (flag == 0) {
      drawMalha();
	}
	if (flag == 1) {
		drawSquare(0, 0, 0, 24, 14, text);
	}
	
	glEnable(GL_TEXTURE_2D);
	//BACK FACE
	glBindTexture(GL_TEXTURE_2D, text);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);    glVertex3i(x + larg / 2, y + alt / 2, z - expr / 2); //A 
	glTexCoord2f(1.0f, 0.0f); 	 glVertex3i(x - larg / 2, y + alt / 2, z - expr / 2);  //B
	glTexCoord2f(1.0f, 1.0f);    glVertex3i(x - larg / 2, y - alt / 2, z - expr / 2); //C
	glTexCoord2f(0.0f, 1.0f);  	 glVertex3i(x + larg / 2, y - alt / 2, z - expr / 2); //D
	glEnd();
	//Right FACE
	glBindTexture(GL_TEXTURE_2D, text);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);    glVertex3i(x + larg / 2, y + alt / 2, z - expr / 2); //A 
	glTexCoord2f(1.0f, 0.0f); 	 glVertex3i(x + larg / 2, y + alt / 2, z + expr / 2);  //B
	glTexCoord2f(1.0f, 1.0f);    glVertex3i(x + larg / 2, y - alt / 2, z + expr / 2); //C
	glTexCoord2f(0.0f, 1.0f);  	 glVertex3i(x + larg / 2, y - alt / 2, z - expr / 2); //D
	glEnd();
	//LefT FACE
	glBindTexture(GL_TEXTURE_2D, text);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);    glVertex3i(x - larg / 2, y + alt / 2, z - expr / 2); //A 
	glTexCoord2f(1.0f, 0.0f); 	 glVertex3i(x - larg / 2, y + alt / 2, z + expr / 2);  //B
	glTexCoord2f(1.0f, 1.0f);    glVertex3i(x - larg / 2, y - alt / 2, z + expr / 2); //C
	glTexCoord2f(0.0f, 1.0f);  	 glVertex3i(x - larg / 2, y - alt / 2, z - expr / 2); //D
	glEnd();
	//BOTTOM FACE
	glBindTexture(GL_TEXTURE_2D, text);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);    glVertex3i(x - larg / 2, y - alt / 2, z - expr / 2); //A 
	glTexCoord2f(1.0f, 0.0f); 	 glVertex3i(x - larg / 2, y - alt / 2, z + expr / 2);  //B
	glTexCoord2f(1.0f, 1.0f);    glVertex3i(x + larg / 2, y - alt / 2, z + expr / 2); //C
	glTexCoord2f(0.0f, 1.0f);  	 glVertex3i(x + larg / 2, y - alt / 2, z - expr / 2); //D
	glEnd();
	//TOP FACE
	glBindTexture(GL_TEXTURE_2D, text);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);    glVertex3i(x - larg / 2, y + alt / 2, z - expr / 2); //A 
	glTexCoord2f(1.0f, 0.0f); 	 glVertex3i(x - larg / 2, y + alt / 2, z + expr / 2);  //B
	glTexCoord2f(1.0f, 1.0f);    glVertex3i(x + larg / 2, y + alt / 2, z + expr / 2); //C
	glTexCoord2f(0.0f, 1.0f);  	 glVertex3i(x + larg / 2, y + alt / 2, z - expr / 2); //D
	glEnd();

	glDisable(GL_TEXTURE_2D);
}
void drawCube(float x, float y, float z, float larg, float alt, float expr, float red, float green, float blue) {
	glBegin(GL_QUADS);
	// Front face 

	glColor3f(red, green, blue);
	glVertex3f(x + larg / 2, y + alt / 2, z + expr / 2);
	glVertex3f(x - larg / 2, y + alt / 2, z + expr / 2);
	glVertex3f(x - larg / 2, y - alt / 2, z + expr / 2);
	glVertex3f(x + larg / 2, y - alt / 2, z + expr / 2);
	// Back face 
	glColor3f(red, green, blue);
	glVertex3f(x + larg / 2, y + alt / 2, z - expr / 2);
	glVertex3f(x - larg / 2, y + alt / 2, z - expr / 2);
	glVertex3f(x - larg / 2, y - alt / 2, z - expr / 2);
	glVertex3f(x + larg / 2, y - alt / 2, z - expr / 2);

	//Right face
	glColor3f(red, green, blue);
	glVertex3f(x + larg / 2, y + alt / 2, z - expr / 2);
	glVertex3f(x + larg / 2, y + alt / 2, z + expr / 2);
	glVertex3f(x + larg / 2, y - alt / 2, z + expr / 2);
	glVertex3f(x + larg / 2, y - alt / 2, z - expr / 2);
	// Left face
	glColor3f(red, green, blue);
	glVertex3f(x - larg / 2, y + alt / 2, z - expr / 2);
	glVertex3f(x - larg / 2, y + alt / 2, z + expr / 2);
	glVertex3f(x - larg / 2, y - alt / 2, z + expr / 2);
	glVertex3f(x - larg / 2, y - alt / 2, z - expr / 2);
	// Bottom face
	glColor3f(red, green, blue);
	glVertex3f(x - larg / 2, y - alt / 2, z - expr / 2);
	glVertex3f(x - larg / 2, y - alt / 2, z + expr / 2);
	glVertex3f(x + larg / 2, y - alt / 2, z + expr / 2);
	glVertex3f(x + larg / 2, y - alt / 2, z - expr / 2);
	// Top face
	glColor3f(red, green, blue);
	glVertex3f(x - larg / 2, y + alt / 2, z - expr / 2);
	glVertex3f(x - larg / 2, y + alt / 2, z + expr / 2);
	glVertex3f(x + larg / 2, y + alt / 2, z + expr / 2);
	glVertex3f(x + larg / 2, y + alt / 2, z - expr / 2);


	glEnd();

}
void drawDoor(float x, float y, float z, float larg, float alt, float expr) {
	glTranslatef(0.0, 0.0, -16.0);
	glRotatef(angle, 0, 1, 0);
	//glRotatef(-angle/3, 1, 0, 0);
	drawCube(x, y, z, larg, alt, expr, 0.6f, 0.45f, 0.15f);
	drawCube(x + larg / 3, y, z + ((expr / 2) + (((expr * 0.3f) / 2) / 2)), (larg * 0.3f) / 4, (alt * 0.3f) / 8, (expr * 0.3f) / 2, 0.6f, 0.6f, 0.6f);
	drawCube(x + (larg / 3) - (((larg * 0.3f) / 2) / 2), y, z + ((expr / 2) + (((expr * 0.3f) / 2))), (larg * 0.6f) / 4, (alt * 0.3f) / 8, (expr * 0.3f) / 2, 0.4f, 0.4f, 0.4f);

}
void drawController(float x, float y, float z, float larg, float alt, float expr) {

	//glTranslatef(0.0, 0.0, -30);
	glRotatef(-angle, 1, 0, 0);
	glRotatef(-angleCamY, 0,0, 1);
	

	
	

	//BASEE
	//glEnable(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, texture[0]);
	initMaterials();
	drawCubeMATERIAL(x, y, z, larg, alt, expr);
	//glDisable(GL_TEXTURE_2D);

	//BOTÂO VERDE
	
	glPushMatrix();
	glTranslatef(0.0, 0.0, transG);
	GLfloat  GreenButtDif[] = { 0.2 ,1 ,0.2 };
	GLint   GreenButtCoef = 0.6 * 128;
	//glMaterialfv(GL_FRONT, GL_AMBIENT, GreenButtDif);
	//glMaterialfv(GL_FRONT, GL_SPECULAR, GreenButtDif);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, GreenButtDif);
	glMaterialf(GL_FRONT, GL_SHININESS, GreenButtCoef);
	drawCubeMATERIAL(x + (larg / 4), y, z + (expr / 2) + (expr / 4), larg / 5, alt/4, expr / 2);
	//drawCube(x + (larg / 4), y, z + (expr / 2) + (expr / 4), larg / 5, alt / 4, expr / 2, 0.2f, 1.0f, 0.0);
	glPopMatrix();


	//BOTÂO AZUL
	
	glPushMatrix();
	glTranslatef(0.0, 0.0, transB);
	GLfloat  BlueButtDif[] = { 0.2, 0.2 ,1 };
	GLint   BlueButtCoef = 0.6 * 128;
	//glMaterialfv(GL_FRONT, GL_AMBIENT, BlueButtDif);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, BlueButtDif);
	//glMaterialfv(GL_FRONT, GL_SPECULAR, BlueButtDif);
	glMaterialf(GL_FRONT, GL_SHININESS, BlueButtCoef);
	drawCubeMATERIAL(x - (larg / 4), y, z + (expr / 2) + (expr / 4), larg / 5, alt / 4, expr / 2);
	//drawCube(x - (larg / 4), y, z + (expr / 2) + (expr / 4), larg / 5, alt / 4, expr / 2, 0.2f, 0.0f, 1.0f);
	glPopMatrix();


	//MANIPULO
	initMaterials();
	glPushMatrix();
	glRotatef(angleX, 1, 0, 0);
	glRotatef(angleY, 0, 1, 0);
	drawCubeMATERIAL(x , y, z + (expr / 2) + (expr * 1.4f), larg / 8, alt / 6, expr *3);
	//drawCube(x, y, z + (expr / 2) + (expr * 1.4f), larg / 8, alt / 6, expr * 3, 0.4f, 0.4f, 0.4f);


	GLfloat  ManipuloDif[] = { 1 ,0.2 ,0.2 };
	GLint   ManipuloCoef = 0.2 * 128;
	glMaterialfv(GL_FRONT, GL_DIFFUSE, ManipuloDif);
	glMaterialf(GL_FRONT, GL_SHININESS, ManipuloCoef);

	glPushMatrix();
	glTranslatef(0, 0, expr * 4);
	glColor3f(1, 0, 0);
	GLUquadric* quad;
	quad = gluNewQuadric();
	gluSphere(quad, 0.9, 100, 20);
	glPopMatrix();
	glPopMatrix();

	//drawCube(x, y, z, larg, alt, expr, 0.6f, 0.6f, 0.6f);

	/*glPushMatrix();
	glTranslatef(0.0, (expr/2)+0.5f, 0.0);
	glColor3f(1, 0, 0);
	GLUquadric* quad;
	quad = gluNewQuadric();
	gluSphere(quad, 1, 100, 20);*/
	

}
void drawChao() {
	GLfloat  bronzeAmb[] = { 0.2125 ,0.1275 ,0.054 };
	GLfloat  bronzeDif[] = { 0.714 ,0.4284 ,0.18144 };
	GLfloat  bronzeSpec[] = { 0.393548 ,0.271906 ,0.166721 };
	GLint  bronzeCoef = 0.2 * 128;
	glMaterialfv(GL_FRONT, GL_AMBIENT, bronzeAmb);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, bronzeDif);
	glMaterialfv(GL_FRONT, GL_SPECULAR, bronzeSpec);
	glMaterialf(GL_FRONT, GL_SHININESS, bronzeCoef);
	glNormal3f(0, 0, 1);
	glBegin(GL_QUADS);
	glVertex3f(-40, -40, -10.5);
	glVertex3f(40, -40, -10.5);
	glVertex3f(40, 40, -10.5);
	glVertex3f(-40, 40, -10.5);
	glEnd();
	

}
void drawTable() {
	//glPushMatrix();
	//glRotated(-65, 1, 0, 0);

	//drawCubeTEXT(0, 0, -1.5, 24, 14, 2, texture[0]);
	//drawMalha();
	drawTampo(0,0,-1.5,24,14,2, texture[0]);
	drawCubeTEXT(12, 6, -6.5, 1, 1, 8, texture[1]);
	drawCubeTEXT(12, -6, -6.5, 1, 1, 8, texture[1]);
	drawCubeTEXT(-12, -6, -6.5, 1, 1, 8, texture[1]);
	drawCubeTEXT(-12, 6, -6.5, 1, 1, 8, texture[1]);
	//glPopMatrix();
}
void drawTransparent() {
	glEnable(GL_BLEND);
	//glDepthMask(GL_FALSE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GLfloat transparent[4] = { 0.6, 0.8, 1.0, transpCoef };
	glMaterialfv(GL_FRONT, GL_DIFFUSE, transparent);
	drawCubeMATERIAL(0, 0, 3, 9, 7, 6);
	//glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}
void drawParedes(float x, float y, float z, float larg, float alt, float expr, GLuint wall,GLuint teto){
	GLfloat branco[4] = { 1, 1, 1, 1.0 };

	glEnable(GL_TEXTURE_2D);

	

	glBindTexture(GL_TEXTURE_2D, teto);
	glNormal3f(0, 0, -1);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);    glVertex3i(x + larg / 2, y + alt / 2, z + expr / 2); //A 
	glTexCoord2f(1.0f, 0.0f); 	 glVertex3i(x - larg / 2, y + alt / 2, z + expr / 2);  //B
	glTexCoord2f(1.0f, 1.0f);    glVertex3i(x - larg / 2, y - alt / 2, z + expr / 2); //C
	glTexCoord2f(0.0f, 1.0f);  	 glVertex3i(x + larg / 2, y - alt / 2, z + expr / 2); //D
	glEnd();
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, branco);
	glMaterialf(GL_FRONT, GL_SHININESS, 128);
	
	
	//Right FACE
	glNormal3f(-1, 0, 0);
	glBindTexture(GL_TEXTURE_2D, wall);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);    glVertex3i(x + larg / 2, y + alt / 2, z - expr / 2); //A 
	glTexCoord2f(1.0f, 0.0f); 	 glVertex3i(x + larg / 2, y + alt / 2, z + expr / 2);  //B
	glTexCoord2f(1.0f, 1.0f);    glVertex3i(x + larg / 2, y - alt / 2, z + expr / 2); //C
	glTexCoord2f(0.0f, 1.0f);  	 glVertex3i(x + larg / 2, y - alt / 2, z - expr / 2); //D
	glEnd();
	//LefT FACE
	glNormal3f(1, 0, 0);
	glBindTexture(GL_TEXTURE_2D, wall);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);    glVertex3i(x - larg / 2, y + alt / 2, z - expr / 2); //A 
	glTexCoord2f(1.0f, 0.0f); 	 glVertex3i(x - larg / 2, y + alt / 2, z + expr / 2);  //B
	glTexCoord2f(1.0f, 1.0f);    glVertex3i(x - larg / 2, y - alt / 2, z + expr / 2); //C
	glTexCoord2f(0.0f, 1.0f);  	 glVertex3i(x - larg / 2, y - alt / 2, z - expr / 2); //D
	glEnd();
	//BOTTOM FACE
	glNormal3f(0, 1, 0);
	glBindTexture(GL_TEXTURE_2D, wall);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);    glVertex3i(x - larg / 2, y - alt / 2, z - expr / 2); //A 
	glTexCoord2f(1.0f, 0.0f); 	 glVertex3i(x - larg / 2, y - alt / 2, z + expr / 2);  //B
	glTexCoord2f(1.0f, 1.0f);    glVertex3i(x + larg / 2, y - alt / 2, z + expr / 2); //C
	glTexCoord2f(0.0f, 1.0f);  	 glVertex3i(x + larg / 2, y - alt / 2, z - expr / 2); //D
	glEnd();
	//TOP FACE
	glNormal3f(0, -1, 0);
	glBindTexture(GL_TEXTURE_2D, wall);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);    glVertex3i(x - larg / 2, y + alt / 2, z - expr / 2); //A 
	glTexCoord2f(1.0f, 0.0f); 	 glVertex3i(x - larg / 2, y + alt / 2, z + expr / 2);  //B
	glTexCoord2f(1.0f, 1.0f);    glVertex3i(x + larg / 2, y + alt / 2, z + expr / 2); //C
	glTexCoord2f(0.0f, 1.0f);  	 glVertex3i(x + larg / 2, y + alt / 2, z - expr / 2); //D
	glEnd();

	glDisable(GL_TEXTURE_2D);

}



void display() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();


	glTranslatef(0.0, 0.0, -20);
	drawController(0, 0, 0, 8, 5, 1);
	drawTable();
	//drawCubeTEXT(0, 0, 0, 80, 80, 50, texture[0]);
	drawParedes(0, 0, 0, 80, 80, 50, texture[2], texture[3]);
	drawChao();
	drawTransparent();
	//drawMalha();
	
	//glPopMatrix();
	//drawChao();
	/*glTranslatef(0.0, 0.0, -16.0);
	glRotatef(angle, 0, 0, 1);
	glColor3f(1, 0, 0);
	GLUquadric* quad;
	quad = gluNewQuadric();
	gluSphere(quad, 7, 100, 20);*/

	reshape(500, 500);
	initLights();



	glutSwapBuffers();
	glFlush();
}

void reshape(int w, int h) {
	/*glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



    glViewport(0, 0, wScreen, hScreen);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(angZoom, (float)wScreen / hScreen, 0.1, profundidadeFim);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(xCamera,yCamera,zCamera, xCamTo, yCamTo, zCamTo, 0, 1, 0);
*/
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(test, 1, 0.1, 90.0);
	
	//gluLookAt(xCamera, yCamera, 12, 0, 0, 0, 0, 1, 0);
	glMatrixMode(GL_MODELVIEW);
	
}

void init() {
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(1.0, 1.0, 1.0, 1.0);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);


	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	initTexturas();
	initLights();
	glEnable(GL_DEPTH_TEST);
}

void timer(int) {

	int state = 0;
	glutPostRedisplay();
	glutTimerFunc(1000 / 60, timer, 0);
	if (GetKeyState('1') & 0x8000)
	{
          if (flag == 0) {
			  flag = 1;

		}
		  else {
			  flag = 0;
		  }

	}
	if (GetKeyState('V') & 0x8000)
	{

		angleCamY++;

	}
	if (GetKeyState('C') & 0x8000)
	{

		angleCamY--;

	}
	if (GetKeyState('X') & 0x8000)
	{
		
			angle++;
		
	}
	if (GetKeyState('Z') & 0x8000)
	{
		
			angle--;
	
	}
	if (GetKeyState('I') & 0x8000)
	{
		if (angleF < 90) {
			angleF++;
		}
	}
	if (GetKeyState('U') & 0x8000)
	{
		if (angleF >0 ) {
			angleF--;
		}
	}
	if (GetKeyState('H') & 0x8000)
	{
		if (light1State == 1) {
			glDisable(GL_LIGHT1);
			light1State = 0;
		}
		else {
			if (light1State == 0) {
				glEnable(GL_LIGHT1);
				light1State = 1;
			}
		}


	}
	if (GetKeyState('G') & 0x8000)
	{
		if (light0State == 1) {
			glDisable(GL_LIGHT0);
			light0State = 0;
		}
		else {
			if (light0State == 0) {
				glEnable(GL_LIGHT0);
				light0State = 1;
			}
		}


	}
	//angle = angle + vel;
	/*if (test >= 1) {
		state = 1;
	}
	else {
		if (test <= 0) {
			state = 0;
		}
	}

	if (state == 0) {
		test = test + 0.01f;
	}
	if (state == 1) {
		test = test - 0.01f;
	}*/
	if (GetKeyState('L') & 0x8000)
	{
		if (transpCoef < 1) {
			transpCoef = transpCoef + 0.03;
		}


	}
	if (GetKeyState('K') & 0x8000)
	{
		if (transpCoef > 0) {
			transpCoef = transpCoef - 0.03;
		}


	}

	if (GetKeyState('P') & 0x8000)
	{
		if (test > 0) {
			test = test - 1;
		}


	}
	if (GetKeyState('O') & 0x8000)
	{
		if (test < 180) {
			test = test + 1;
		}

	}
	if (GetKeyState('N') & 0x8000)
	{
		transG = -0.3f;
		FluzR = 0.0;
		FluzG = 1.0;
		FluzB = 0.0;
		
		//glClearColor(0.0, 1.0, 0.0, 1.0);
	}
	else {
		transG = 0;
		
		if (GetKeyState('B') & 0x8000)
		{
			transB = -0.3f;
			FluzR = 0.0;
			FluzG = 0.0;
			FluzB = 1.0;

			//glClearColor(0.0, 0.0, 1.0, 1.0);
		}
		else {
			transB = 0;
			FluzR = 1.0;
			FluzG = 1.0;
			FluzB = 1.0;

		}
	}

	

	if (GetKeyState('W') & 0x8000)
	{
		angleX = -15;
		posY = posY + 0.5;
	}
	else {
		if (GetKeyState('S') & 0x8000)
		{
			angleX = 15;
			posY = posY - 0.5;
		}
		else {
			angleX = 0;
		}
	}





	if (GetKeyState('A') & 0x8000)
	{
		angleY = -15;
		posX = posX - 0.5;
	}
	else {
		if (GetKeyState('D') & 0x8000)
		{
			angleY = 15;
			posX = posX + 0.5;
		}
		else {
			angleY = 0;
		}
	}






}



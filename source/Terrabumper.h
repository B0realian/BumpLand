#pragma once
#define GLEW_STATIC
#include "glew.h"
#include <vector>
#include <string>


class Terrabumper
{
public:
	bool LoadTGA(const std::string &filename);
	bool LoadTGA(const std::string &filename, int heightFactor);


	int imageWidth;
	int imageHeight;
	int bytesPerPixel;
	int imageSize;
	int triangles;
	float heightScale = 0.00005f;
	std::vector<float> vertices;
	std::vector<unsigned int> vertexIndex;

private:
	bool TGAHeaderCheck(unsigned char header[18]);
	void Terraform(const std::vector<unsigned char> &data, int stride);
	void SaveFile();


	bool tgaCompressed;
	unsigned char tgaHeader[18];
	std::vector<unsigned char> imageData;

};
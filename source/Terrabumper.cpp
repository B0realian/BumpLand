#include "Terrabumper.h"
#include <iostream>
#include <fstream>


bool Terrabumper::LoadTGA(const std::string &filename)
{
	std::ifstream imageFile;
	imageFile.open(filename.c_str(), std::ios::binary);
	imageFile.read((char*)&tgaHeader, 18);
	
	if (!TGAHeaderCheck(tgaHeader))
	{
		imageFile.close();
		std::cout << "Failed to read header of tga-file." << std::endl;
		return false;
	}
	if (tgaCompressed)
	{
		imageFile.close();
		std::cout << "Compressed tga-files not supported." << std::endl;
		return false;
	}

	imageData.reserve(imageSize);
	unsigned char tempContainer = 0;
	imageFile.seekg(18, std::ios::beg);
	for (int i = 0; i < imageSize; i++)
	{
		imageFile.read((char*)&tempContainer, 1);
		imageData.push_back(tempContainer);
	}

	Terraform(imageData, bytesPerPixel);

	imageFile.close();
	SaveFile();
	return true;
}

bool Terrabumper::TGAHeaderCheck(unsigned char header[18])
{
	int sum = 0;
	for (int i = 0; i < 12; i++)
		sum += header[i];
	if (!(sum == header[2]))
		return false;
	
	if (header[2] == 2)
		tgaCompressed = false;
	else if (header[2] == 10)
		tgaCompressed = true;
	else
		return false;
	
	imageWidth = header[13] * 256 + header[12];
	imageHeight = header[15] * 256 + header[14];
	bytesPerPixel = header[16] / 8;
	imageSize = imageWidth * imageHeight * bytesPerPixel;

	return true;
}

void Terrabumper::Terraform(const std::vector<unsigned char> &data, int stride)
{
	int rowLength = imageWidth * stride;
	float halfWidth = imageWidth / 2.f;
	float doubleWidth = imageWidth * 2.f;
	int x = 0;
	int y = 0;
	int z = 0;
	vertices.reserve(imageWidth * imageHeight * sizeof(float) * 3);
	vertexIndex.reserve((imageWidth - 1) * (imageHeight - 1) * 6 * sizeof(unsigned int));

	for (int i = 0; i < (rowLength * imageHeight); i += rowLength)
	{
		for (int z = 0; z < rowLength; z += stride)
		{
			vertices.push_back((x - halfWidth) / doubleWidth);
			vertices.push_back((y - halfWidth) / doubleWidth);
			vertices.push_back(data[z + i] / doubleWidth);
			x++;
		}
		y++;
	}

	for (int i = 0; i < (imageHeight - 1); i++)
	{
		for (int j = 0; j < (imageWidth - 1); j++)
		{
			vertexIndex.push_back(j + (i * imageWidth));
			vertexIndex.push_back(j + 1 + (i * imageWidth));
			vertexIndex.push_back(j + imageWidth + (i * imageWidth));
			vertexIndex.push_back(j + 1 + (i * imageWidth));
			vertexIndex.push_back(j + imageWidth + (i * imageWidth));
			vertexIndex.push_back(j + imageWidth + 1 + (i * imageWidth));
		}
	}
}

void Terrabumper::SaveFile()
{
	std::ofstream saveStream;
	saveStream.open("logs/vertAndIndexDebug.log");
	for (int i = 0; i < imageWidth; i += 3)
	{
		saveStream << vertices[i] << ", " << vertices[i + 1] << ", " << vertices[i + 2] << ",  ; " << vertexIndex[i] << ", " << vertexIndex[i + 1] << ", " << vertexIndex[i + 2] << ",\n";
	}
	saveStream.close();
}
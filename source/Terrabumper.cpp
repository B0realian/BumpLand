#include "Terrabumper.h"
#include <iostream>
#include <fstream>
#include <iomanip>

bool Terrabumper::LoadTGA(const std::string &filename)
{
	return (LoadTGA(filename, 1));
}

bool Terrabumper::LoadTGA(const std::string &filename, int heightFactor)
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

	heightScale *= heightFactor;
	Terraform(imageData, bytesPerPixel);

	imageFile.close();
	// SaveFile();			// WARNING!!! This might write some pretty big log files.
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
			float average = ((float)data[z + i] + (float)data[z + i + 1] + (float)data[z + i + 2]) / 3;
			vertices.push_back((x - halfWidth) / imageWidth);
			vertices.push_back(average * heightScale);
			vertices.push_back((y - halfWidth) / imageWidth);
			x++;
		}
		x = 0;
		y++;
	}
	//std::cout << "Vertices length: " << vertices.size() << std::endl;

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
	triangles = vertexIndex.size() / 3;
	//std::cout << "Index length: " << vertexIndex.size() << std::endl;
	//std::cout << "Amount of triangles: " << triangles << std::endl;
}

void Terrabumper::SaveFile()
{
	int width = 10;
	std::ofstream vertStream;
	vertStream.open("logs/vertDebug.log");
	for (int i = 0; i < vertices.size(); i += 3)
		vertStream << std::setw(width) << vertices[i] << ", " << std::setw(width) << vertices[i + 1] << ", " << std::setw(width) << vertices[i + 2] << ",\n";
	vertStream.close();
	std::ofstream indexStream;
	indexStream.open("logs/indexDebug.log");
	for (int i = 0; i < vertexIndex.size(); i += 3)
		indexStream << std::setw(width) << vertexIndex[i] << ", " << std::setw(width) << vertexIndex[i + 1] << ", " << std::setw(width) << vertexIndex[i + 2] << ",\n";
	indexStream.close();
}
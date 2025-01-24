#include <iostream>
#include <string>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "stb_image_write.h"

#include "Image.h"

// This allows you to skip the `std::` in front of C++ standard library
// functions. You can also say `using std::cout` to be more selective.
// You should never do this in a header file.
using namespace std;


struct Vec2 {
	float x, y;
};

double RANDOM_COLORS[7][3] = {
	{0.0000,    0.4470,    0.7410},
	{0.8500,    0.3250,    0.0980},
	{0.9290,    0.6940,    0.1250},
	{0.4940,    0.1840,    0.5560},
	{0.4660,    0.6740,    0.1880},
	{0.3010,    0.7450,    0.9330},
	{0.6350,    0.0780,    0.1840},
};

//function to compute the bounding box for 3 vertices as a triangle
void computeTriangleBoundingBox(const vector<float>& posBuf, size_t startIdx, float& minX, float& minY, float& maxX, float& maxY) {
	minX = posBuf[startIdx];
	minY = posBuf[startIdx + 1];
	maxX = posBuf[startIdx];
	maxY = posBuf[startIdx + 1];

	for (int i = 0; i < 3; ++i) {
		float x = posBuf[startIdx + i * 3];
		float y = posBuf[startIdx + i * 3 + 1];
		if (x < minX) minX = x;
		if (y < minY) minY = y;
		if (x > maxX) maxX = x;
		if (y > maxY) maxY = y;
	}
}

//function to compute bounding box for whole object
void computeBoundingBox(const vector<float>& posBuf, float& minX, float& minY, float& maxX, float& maxY)
{
	minX = posBuf[0];
	minY = posBuf[1];
	maxX = posBuf[0];
	maxY = posBuf[1];
	for (size_t i = 0; i < posBuf.size(); i += 3) 
	{
		float x = posBuf[i];
		float y = posBuf[i + 1];
		if (x < minX) minX = x;
		if (y < minY) minY = y;
		if (x > maxX) maxX = x;
		if (y > maxY) maxY = y;
	}
}

Vec2 projectToImage(float x, float y, float scale, const Vec2& translation) 
{
	return { scale * x + translation.x, scale * y + translation.y };
}

int main(int argc, char **argv)
{

	if(argc < 5) {
		cerr << "Inusfficient amount of arguments" << endl;
		return 1;
	}

	string meshName = argv[1];
	string outputName = argv[2];
	int imageWidth = atoi(argv[3]);
	int imageHeight = atoi(argv[4]);
	int task = atoi(argv[5]);


	// Load geometry
	vector<float> posBuf; // list of vertex positions
	vector<float> norBuf; // list of vertex normals
	vector<float> texBuf; // list of vertex texture coords
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	string warnStr, errStr;
	bool rc = tinyobj::LoadObj(&attrib, &shapes, &materials, &warnStr, &errStr, meshName.c_str());
	if(!rc) {
		cerr << errStr << endl;
	} else {
		// Some OBJ files have different indices for vertex positions, normals,
		// and texture coordinates. For example, a cube corner vertex may have
		// three different normals. Here, we are going to duplicate all such
		// vertices.
		// Loop over shapes
		for(size_t s = 0; s < shapes.size(); s++) {
			// Loop over faces (polygons)
			size_t index_offset = 0;
			for(size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
				size_t fv = shapes[s].mesh.num_face_vertices[f];
				// Loop over vertices in the face.
				for(size_t v = 0; v < fv; v++) {
					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					posBuf.push_back(attrib.vertices[3*idx.vertex_index+0]);
					posBuf.push_back(attrib.vertices[3*idx.vertex_index+1]);
					posBuf.push_back(attrib.vertices[3*idx.vertex_index+2]);
					if(!attrib.normals.empty()) {
						norBuf.push_back(attrib.normals[3*idx.normal_index+0]);
						norBuf.push_back(attrib.normals[3*idx.normal_index+1]);
						norBuf.push_back(attrib.normals[3*idx.normal_index+2]);
					}
					if(!attrib.texcoords.empty()) {
						texBuf.push_back(attrib.texcoords[2*idx.texcoord_index+0]);
						texBuf.push_back(attrib.texcoords[2*idx.texcoord_index+1]);
					}
				}
				index_offset += fv;
				// per-face material (IGNORE)
				shapes[s].mesh.material_ids[f];
			}
		}
	}
	cout << "Number of vertices: " << posBuf.size()/3 << endl;

	float minX, minY, maxX, maxY;
	computeBoundingBox(posBuf, minX, minY, maxX, maxY);

	float bboxWidth = maxX - minX;
	float bboxHeight = maxY - minY;
	float scale = std::min(static_cast<float>(imageWidth) / bboxWidth, static_cast<float>(imageHeight) / bboxHeight);
	Vec2 translation = {
		static_cast<float>(imageWidth) / 2.0f - scale * (minX + maxX) / 2.0f,
		static_cast<float>(imageHeight) / 2.0f - scale * (minY + maxY) / 2.0f
	};

	std::vector<unsigned char> image(imageWidth * imageHeight * 3, 0);

	for (size_t i = 0; i < posBuf.size(); i += 9) { // Iterate through triangles and assign colour
		int colorIndex = (i / 9) % 7;
		const auto& color = RANDOM_COLORS[colorIndex];

		// Compute bounding box for the current triangle
		float triMinX, triMinY, triMaxX, triMaxY;
		computeTriangleBoundingBox(posBuf, i, triMinX, triMinY, triMaxX, triMaxY);

		// Project bounding box corners
		Vec2 projectedMin = projectToImage(triMinX, triMinY, scale, translation);
		Vec2 projectedMax = projectToImage(triMaxX, triMaxY, scale, translation);

		// Fill the bounding box area with color
		for (int y = static_cast<int>(projectedMin.y); y <= static_cast<int>(projectedMax.y); ++y) {
			for (int x = static_cast<int>(projectedMin.x); x <= static_cast<int>(projectedMax.x); ++x) {
				if (x >= 0 && x < imageWidth && y >= 0 && y < imageHeight) {
					int pixelIndex = (y * imageWidth + x) * 3;
					image[pixelIndex] = static_cast<unsigned char>(color[0] * 255);
					image[pixelIndex + 1] = static_cast<unsigned char>(color[1] * 255);
					image[pixelIndex + 2] = static_cast<unsigned char>(color[2] * 255);
				}
			}
		}
	}

	if (stbi_write_png(outputName.c_str(), imageWidth, imageHeight, 3, image.data(), imageWidth * 3)) {
		cout << "Output written to " << outputName << "\n";
	}
	else {
		cerr << "Failed to write output file " << outputName << "\n";
		return 1;
	}

	return 0;
}

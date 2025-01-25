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


struct Point {
	float x, y;
};

struct Vertex {
	float x, y, z;
};

struct Tri {
	Vertex a, b, c;
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

Point projectToImage(float x, float y, float scale, const Point& translation) 
{
	return { scale * x + translation.x, scale * y + translation.y };
}

float edgeFunction(const Vertex& a, const Vertex& b, const Vertex& c) {
	return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
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
	vector<Point> frameBuf; //buffer for each pixel colour
	vector<Vertex> ZBuf; //buffer for each pixel z value
	vector<Vertex> Vertices;
	vector<Tri> Triangles;
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

	for (size_t i = 0; i < posBuf.size(); i += 3) 
	{
		Vertex temp;
		temp.x = posBuf[i];
		temp.y = posBuf[i + 1];
		temp.z = posBuf[i + 2];
		Vertices.push_back(temp);
	}

	for (size_t i = 0; i < posBuf.size(); i += 9)
	{
		Vertex temp1;
		Vertex temp2;
		Vertex temp3;
		Tri angle;

		temp1.x = posBuf[i];
		temp1.y = posBuf[i + 1];
		temp1.z = posBuf[i + 2];
		temp2.x = posBuf[i + 3];
		temp2.y = posBuf[i + 4];
		temp2.z = posBuf[i + 5];
		temp3.x = posBuf[i + 6];
		temp3.y = posBuf[i + 7];
		temp3.z = posBuf[i + 8];

		angle = { temp1, temp2, temp3 };

		Triangles.push_back(angle);
	}


	float minX, minY, maxX, maxY;
	computeBoundingBox(posBuf, minX, minY, maxX, maxY);

	float bboxWidth = maxX - minX;
	float bboxHeight = maxY - minY;
	float scale = min(static_cast<float>(imageWidth) / bboxWidth, static_cast<float>(imageHeight) / bboxHeight);
	Point translation = {
		static_cast<float>(imageWidth) / 2.0f - scale * (minX + maxX) / 2.0f,
		static_cast<float>(imageHeight) / 2.0f - scale * (minY + maxY) / 2.0f
	};

	vector<unsigned char> image(imageWidth * imageHeight * 3, 0);

	for (size_t i = 0; i < Triangles.size(); i++) { // Iterate through triangles and assign colour
		int colorIndex = i % 7;
		const auto& color = RANDOM_COLORS[colorIndex];

		Point a = projectToImage(Triangles[i].a.x, Triangles[i].a.y, scale, translation);
		Point b = projectToImage(Triangles[i].b.x, Triangles[i].b.y, scale, translation);
		Point c = projectToImage(Triangles[i].c.x, Triangles[i].c.y, scale, translation);

		// Compute bounding box for the current triangle
		float triMinX, triMinY, triMaxX, triMaxY;
		//computeTriangleBoundingBox(posBuf, i, triMinX, triMinY, triMaxX, triMaxY);
		triMinX = min(min(a.x, b.x), c.x);
		triMinY = min(min(a.y, b.y), c.y);
		triMaxX = max(max(a.x, b.x), c.x);
		triMaxY = max(max(a.y, b.y), c.y);

		// Project bounding box corners
		//Point projectedMin = projectToImage(triMinX, triMinY, scale, translation);
		//Point projectedMax = projectToImage(triMaxX, triMaxY, scale, translation);

		for (int y = static_cast<int>(triMinY); y < static_cast<int>(triMaxY); y++)
		{
			for (int x = static_cast<int>(triMinX); x < static_cast<int>(triMaxX); x++)
			{
				Vertex pixel = { static_cast<float>(x), static_cast<float>(y), 0.0f };
				float ABP = edgeFunction(Vertex{ a.x, a.y, 0.0f }, Vertex{ b.x, b.y, 0.0f }, pixel);
				float BCP = edgeFunction(Vertex{ b.x, b.y, 0.0f }, Vertex{ c.x, c.y, 0.0f }, pixel);
				float CAP = edgeFunction(Vertex{ c.x, c.y, 0.0f }, Vertex{ a.x, a.y, 0.0f }, pixel);

				int flippedY = imageHeight - 1 - y;
				if (x >= 0 && x < imageWidth && flippedY >= 0 && flippedY < imageHeight && task == 1)
				{
					int pixelIndex = (flippedY * imageWidth + x) * 3;
					image[pixelIndex] = static_cast<unsigned char>(color[0] * 255);
					image[pixelIndex + 1] = static_cast<unsigned char>(color[1] * 255);
					image[pixelIndex + 2] = static_cast<unsigned char>(color[2] * 255);
				}


				if (ABP >= -1e-5 && BCP >= -1e-5 && CAP >= -1e-5)
				{
					int pixelIndex = (flippedY * imageWidth + x) * 3;
					image[pixelIndex] = static_cast<unsigned char>(color[0] * 255);
					image[pixelIndex + 1] = static_cast<unsigned char>(color[1] * 255);
					image[pixelIndex + 2] = static_cast<unsigned char>(color[2] * 255);
				}

				/*if (P.x >= 0 && P.x < imageWidth && flippedY >= 0 && flippedY < imageHeight)
				{
					int pixelIndex = (flippedY * imageWidth + P.x) * 3;
					image[pixelIndex] = static_cast<unsigned char>(color[0] * 255);
					image[pixelIndex + 1] = static_cast<unsigned char>(color[1] * 255);
					image[pixelIndex + 2] = static_cast<unsigned char>(color[2] * 255);
				}*/
			}
		}

		/*for (int y = static_cast<int>(projectedMin.y); y <= static_cast<int>(projectedMax.y); ++y) {
			for (int x = static_cast<int>(projectedMin.x); x <= static_cast<int>(projectedMax.x); ++x) {
				int flippedY = imageHeight - 1 - y; 
				if (x >= 0 && x < imageWidth && flippedY >= 0 && flippedY < imageHeight) {
					int pixelIndex = (flippedY * imageWidth + x) * 3;
					image[pixelIndex] = static_cast<unsigned char>(color[0] * 255);
					image[pixelIndex + 1] = static_cast<unsigned char>(color[1] * 255);
					image[pixelIndex + 2] = static_cast<unsigned char>(color[2] * 255);
				}
			}
		}*/
	}

	//init frame buffer to (0,0,0)
	//init zbuf to -99999999999999999
	//for all triangles
	//	for all pixels in the tri bounding box
	//		compute pixel's barycentric coords
	//		if inside
	//			compute pixel's z-coords (A.z * a + B.z * b + C.z + c)
	//			compute pixel's RGB color
	//			if z > curr z from Zbuffer
	//				write RGB to Fbuf
	//				write z to Zbuf
	//			end
	//		end
	//	end
	//end
	//make sure to check if val at Zbuf is bigger when trying to draw overlapping points



	if (stbi_write_png(outputName.c_str(), imageWidth, imageHeight, 3, image.data(), imageWidth * 3)) {
		cout << "Output written to " << outputName << "\n";
	}
	else {
		cerr << "Failed to write output file " << outputName << "\n";
		return 1;
	}

	return 0;
}

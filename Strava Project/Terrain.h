#pragma once
#include <Object.h>
#include <glm.hpp>
#include <memory>
#define glNull (~0)
struct face {
	glm::uvec3 tri1, tri2;
};
class Terrain : public Object
{
protected:
	unsigned int vao, vbo, ebo;
	std::unique_ptr<unsigned int[]> indices;
	size_t indicesSize, totalVertices;
	glm::vec3 color = { -1, -1, -1 };
	unsigned int tex = glNull;
public:
	/**
	* @param points, flattened 2d array of size count x count
	*/
	Terrain(glm::vec3 * points, size_t count);
	virtual ~Terrain();
	void draw(const Shader * s) const override;
	void setColor(glm::vec3 color);
	void setTexture(const unsigned int tex) { this->tex = tex; }
};
class Trail : public Object
{
protected:
	unsigned int vao, vbo, fbo;
	unsigned int tex;
	size_t totalVertices;
	glm::vec3 color = { -1, -1, -1 };
	glm::vec4 bg;
public:
	static const int textureWidth = 1920, textureHeight = 1080;
public:
	Trail(glm::vec3 * points, size_t numPoints);
	~Trail();
	void draw(const Shader * s) const override;
	void setColor(glm::vec3 color);
	const unsigned int getTexture() const { return tex; }
	void setBgColor(glm::vec4 color) { bg = color; }
};
float map(float minx, float maxx, float x, float miny, float maxy);
float sample(glm::vec3 * plane, size_t planeSize, float x, float z);


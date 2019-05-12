#pragma once
#include <Object.h>
#include <glm.hpp>
#include <memory>
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
public:
	/**
	* @param points, flattened 2d array of size count x count
	*/
	Terrain(glm::vec3 * points, size_t count);
	virtual ~Terrain();
	void draw(const Shader * s) const override;
	void setColor(glm::vec3 color);
};


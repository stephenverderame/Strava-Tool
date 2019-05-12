#include "Terrain.h"

Terrain::Terrain(glm::vec3 * points, size_t count)
{
	assert(count >= 2 && "Need at least 4  points!");
	auto normals = std::make_unique<glm::vec3[]>(count * count);
	for (size_t i = 0; i < count; ++i) {
		for (size_t j = 0; j < count; ++j) {
			size_t z = i == 0 ? 1 : i;
			size_t x = j == 0 ? 1 : j;
			float hl = points[z * count + x - 1].y;
			float hr = points[z * count + x + 1].y;
			float hd = points[(z + 1) * count + x].y;
			float hu = points[(z - 1) * count + x].y;
			normals[i * count + j] = glm::normalize(glm::vec3(hl - hr, 2.0f, hd - hu));
		}
	}
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	auto buffer = std::make_unique<glm::vec3[]>(count * count * 2);
	for (size_t i = 0; i < count * count; ++i) {
		buffer[i * 2] = points[i];
		buffer[i * 2 + 1] = normals[i];
	}
	glBufferData(GL_ARRAY_BUFFER, count * count * 2 * sizeof(glm::vec3), buffer.get(), GL_STATIC_DRAW);
	totalVertices = count * count;
	size_t rects = (count - 1) * (count - 1); 
	indicesSize = rects * 6;
	indices = std::make_unique<unsigned int[]>(rects * 6);
	unsigned int index = 0;
	for (size_t i = 0; i < count - 1; ++i) {
		for (size_t j = 0; j < count - 1; ++j) {
			unsigned int base = i * count + j;
			indices[index] = base;
			indices[index + 1] = base + 1;
			indices[index + 2] = base + count;

			indices[index + 3] = base + 1;
			indices[index + 4] = base + count + 1;
			indices[index + 5] = base + count;
			index += 6;
		}
	}
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, rects * sizeof(unsigned int) * 6, indices.get(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(glm::vec3), (void*)sizeof(glm::vec3));
	glEnableVertexAttribArray(1);

}

Terrain::~Terrain()
{
	glDeleteBuffers(1, &ebo);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}

void Terrain::draw(const Shader * s) const
{
	if (color != glm::vec3{ -1, -1, -1 })
		s->setVec4("color", glm::vec4(color, 1.0));
	s->setMat4("model", calcModel());
	s->setFloat("shininess", shininess);
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, indicesSize, GL_UNSIGNED_INT, 0);
//	glDrawArrays(GL_POINTS, 0, totalVertices);
//	glDrawArrays(GL_LINE_STRIP, 0, totalVertices);
}

void Terrain::setColor(glm::vec3 color)
{
	this->color = std::move(color);
}

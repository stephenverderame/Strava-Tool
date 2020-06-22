#define _CRT_SECURE_NO_WARNINGS
#include "Terrain.h"
#include <Shape.h>

float map(float minx, float maxx, float x, float miny, float maxy)
{
	return (maxy - miny) * (x - minx) / (maxx - minx) + miny;
}

float sample(glm::vec3 * plane, size_t planeSize, float x, float z)
{
	float h00 = plane[planeSize * (int)z + (int)x].y;
	float h10 = plane[planeSize * (int)z + (int)x + 1].y;
	float h01 = plane[planeSize * ((int)z + 1) + (int)x].y;
	float h11 = plane[planeSize * ((int)z + 1) + (int)x + 1].y;

	float hx0 = h00 * (x - floor(x)) + h10 * (ceil(x) - x);
	float hx1 = h01 * (x - floor(x)) + h11 * (ceil(x) - x);
	return hx0 * (z - floor(z)) + hx1 * (ceil(z) - z);
}

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
	auto texCoords = std::make_unique<glm::vec3[]>(count * count);
	for (size_t i = 0; i < count; ++i) {
		for (size_t j = 0; j < count; ++j) {
			glm::vec3 & coord = texCoords[i * count + j];
			coord.x = map(0, count, j, 0, 1);
			coord.y = map(0, count, i, 0, 1);
		}
	}
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	auto buffer = std::make_unique<glm::vec3[]>(count * count * 3);
	for (size_t i = 0; i < count * count; ++i) {
		buffer[i * 3] = points[i];
		buffer[i * 3 + 1] = normals[i];
		buffer[i * 3 + 2] = texCoords[i];
	}
	glBufferData(GL_ARRAY_BUFFER, count * count * 3 * sizeof(glm::vec3), buffer.get(), GL_STATIC_DRAW);
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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec3), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec3), (void*)sizeof(glm::vec3));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(glm::vec3), (void*)(2 * sizeof(glm::vec3)));
	glEnableVertexAttribArray(2);

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
	if (tex != glNull) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);
	}
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

Trail::Trail(glm::vec3 * points, size_t numPoints) : totalVertices(numPoints)
{
/*	for (size_t i = 0; i < numPoints; ++i) {
		float x = map(left, right, points[i].x, 0, planeSize);
		float z = map(btm, top, points[i].z, 0, planeSize);
		points[i].y = sample(plane, planeSize, 0, 0) + 0.001f;
	}*/
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
	GLenum draw = GL_COLOR_ATTACHMENT0;
	glDrawBuffers(1, &draw);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) printf("Framebuffer incomplete!\n");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * numPoints, points, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
	glEnableVertexAttribArray(0);

	float rectData[12] = {
		1.f, 0.f, 1.f,   // top left 
		1.f, 0.f, 0.f,   // bottom left	
		0.f, 0.f, 1.f,    // top right
		0.f, 0.f, 0.f, //btm right
	};
	glGenVertexArrays(1, &rVao);
	glBindVertexArray(rVao);
	glGenBuffers(1, &rVbo);
	glBindBuffer(GL_ARRAY_BUFFER, rVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectData), rectData, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
	glEnableVertexAttribArray(0);

}

Trail::~Trail()
{
	glDeleteTextures(1, &tex);
	glDeleteBuffers(1, &vbo);
	glDeleteFramebuffers(1, &fbo);
	glDeleteVertexArrays(1, &vao);

	glDeleteBuffers(1, &rVbo);
	glDeleteVertexArrays(1, &rVao);
}

void Trail::draw(const Shader * s) const
{
	if (color != glm::vec3{ -1, -1, -1 })
		s->setVec4("color", glm::vec4(color, 1.0));
//	s->setMat4("model", calcModel());
//	s->setFloat("shininess", shininess);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClearColor(bg.r, bg.g, bg.b, bg.a);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	s->setInt("tex", 0);
	bgTex->bind();
	glBindVertexArray(rVao);
	s->setBool("useModel", true);
	s->setMat4("model", model);
	glm::mat4 m;
	m = glm::rotate(m, glm::radians(180.f), glm::vec3(1, 0, 0));
	s->setMat4("texCorrection", m);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	s->setBool("useModel", false);
	glBindVertexArray(vao);
	s->setBool("useColor", true);
	glDrawArrays(GL_LINE_STRIP, 0, totalVertices);
	s->setBool("useColor", false);
	s->setBool("useModel", false);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);
}

void Trail::setColor(glm::vec3 color)
{
	this->color = std::move(color);
}

void Trail::setBgImage(const std::string & data)
{
/*	FILE * f = fopen("testImg.jpg", "wb");
	fwrite(data.c_str(), 1, data.size(), f);
	fclose(f);*/
	texOptions o;
	o.wrap_s = GL_CLAMP_TO_EDGE;
	o.wrap_t = GL_CLAMP_TO_EDGE;
	o.mag = GL_LINEAR;
	o.min = GL_LINEAR;
	bgTex = std::make_unique<Texture>((const unsigned char*)data.c_str(), data.size());
//	printf("Texture: %d %d", bgTex->getWidth(), bgTex->getHeight());
	bgTex->setId(0);
	bgTex->bind();

	model = glm::scale(model, glm::vec3(textureWidth, textureHeight, 1.0));
//	model = glm::rotate(model, glm::radians(180.f), glm::vec3(1.0, 0.0, 0.0));
}


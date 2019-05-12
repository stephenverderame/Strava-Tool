#pragma once
#include <vector>
#include <Https.h>
#include <glm.hpp>
struct coordinate {
	double lat, lon;
	coordinate(double lat, double lon) : lat(lat), lon(lon) {};
	coordinate() : coordinate(0, 0) {};
};
class Polyline
{
public:
	static std::vector<coordinate> decode(const std::string & polyline, int precision = 5);
	static std::string encode(const std::vector<coordinate> & coordinates, int precision = 5);
private:
	static int decodeHelper(const std::string & polyline, int & index);
	static std::vector<char> encodeHelper(const int num);
};

namespace height {
	void getHeights(glm::vec3 * points, size_t num);
}


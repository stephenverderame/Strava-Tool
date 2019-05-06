#pragma once
#include <vector>
#include <Https.h>
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
	struct h_coord {
		float x, y, z;
		h_coord(float lon, float lat, float height) : x(lon), y(lat), z(height) {};
		h_coord(float lat, float lon) : y(lat), x(lon), z(0) {};
		h_coord() : x(0), y(0), z(0) {};
	};
	void getHeights(h_coord * points, size_t num);
}


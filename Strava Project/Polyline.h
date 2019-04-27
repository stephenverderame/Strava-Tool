#pragma once
#include <vector>
struct coordinate {
	double lat, lon;
};
class Polyline
{
public:
	static std::vector<coordinate> decode(const std::string & polyline, int precision = 5);
private:
	static int decodeHelper(const std::string & polyline, int & index);
};


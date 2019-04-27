#include "Polyline.h"
#include <memory>

std::vector<coordinate> Polyline::decode(const std::string & polyline, int precision)
{
	std::vector<coordinate> points;
	int i = 0;
	int lat = 0;
	int lon = 0;
	while (i < polyline.size()) {
		int latdiff = decodeHelper(polyline, i);
		int londiff = decodeHelper(polyline, i);
		lat += latdiff;
		lon += londiff;
		points.push_back({ lat / pow(10, precision), lon / pow(10, precision) });
	}
	return points;
}

int Polyline::decodeHelper(const std::string & polyline, int & index)
{
	int shift = 0, result = 0;
	char bit = 0;
	do {
		bit = polyline[index++] - 63;
		result |= (bit & 0x1f) << shift;
		shift += 5;
	} while (bit >= 0x20);
	return (result & 1) ? ~(result >> 1) : (result >> 1);
}

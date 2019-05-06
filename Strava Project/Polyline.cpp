#define _CRT_SECURE_NO_WARNINGS
#include "Polyline.h"
#include <memory>
#include <array>
#include <sstream>

#include <Https.h>

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
		points.emplace_back(lat / pow(10, precision), lon / pow(10, precision));
	}
	return points;
}

std::string Polyline::encode(const std::vector<coordinate>& coordinates, int precision)
{
	std::vector<char> out;
	double lastLat = 0, lastLon = 0;
	for (auto & c : coordinates) {
		int val = round((c.lat - lastLat) * pow(10, precision));
		std::vector<char> n = encodeHelper(val);
		out.insert(out.end(), n.cbegin(), n.cend());
		val = round((c.lon - lastLon) * pow(10, precision));
		n = encodeHelper(val);
		out.insert(out.end(), n.cbegin(), n.cend());
		lastLat = c.lat;
		lastLon = c.lon;
	}
	out.push_back('\0');
	return std::string(out.data());
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

std::vector<char> Polyline::encodeHelper(const int num)
{
	std::vector<char> buffer;
	int val = num < 0 ? ~(num << 1) : (num << 1);
	int shift = 0;
	bool moreChunks = false;
	do {
		int chunk = (val >> shift);
		shift += 5;
		moreChunks = chunk > 0x20;
		chunk &= 0b11111;
		if (moreChunks)
			chunk |= 0x20;
		chunk += 63;
		buffer.push_back(chunk);
	} while (moreChunks);
	return buffer;
}

void height::getHeights(h_coord * points, size_t num)
{
	
	HttpsClient client("elevation-api.io");
	HttpFrame request;
	request.protocol = "GET";
	request.headers["Host"] = "elevation-api.io";
	request.headers["Connection"] = "keep-alive";
	request.headers["Accept"] = "application/json, */*";
	const char * file = "/api/elevation?points=";
	if (client.connectClient() != 0) printf("Error connecting client!\n");
	std::stringstream ss;
	int loops = num / 10 + 1;
	for (int i = 0; i < loops; ++i) { //max 10 points per request
		ss.str("");
		ss << file;
		bool send = false;
		for (int j = 0; j < 10; ++j) {
			if (i * 10 + j >= num) break;
			ss << "(" << points[i * 10 + j].y << "," << points[i * 10 + j].x << ")";
			send = true;
			if (i * 10 + j + 1 < num) ss << ",";
			else break;
		}
		if (!send) break;
		request.file = ss.str();
		request.composeRequest();
//		printf("Request: %s\n", request.data.c_str());
		client.sendMessage(request);
		HttpFrame response;
		client.getMessage(response);
//		printf("Response: %s\n", response.data.c_str());
		auto searchStart = response.data.cbegin();
		std::smatch match;
		int j = 0;
		while (std::regex_search(searchStart, response.data.cend(), match, std::regex("\"elevation\":"))) {
			size_t pos = match.position() + 12;
			std::string num = response.data.substr(pos + (searchStart - response.data.cbegin()), response.data.find(',', pos + (searchStart - response.data.cbegin())) - (pos + (searchStart - response.data.cbegin())));
			float h = std::stof(num);
			points[i * 10 + j].z = h;
			searchStart = match.suffix().first;
			++j;
		}
	}
}


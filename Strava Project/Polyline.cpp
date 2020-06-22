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

void height::getHeights(glm::vec3 * points, size_t num)
{
	
	HttpsClient client("elevation-api.io");
	HttpFrame request;
	request.protocol = "GET";
	request.headers["Host"] = "elevation-api.io";
	request.headers["Connection"] = "keep-alive";
	request.headers["Accept"] = "application/json, */*";
//	request.headers["ELEVATION-API-KEY"] = "h0edaS8ftgo30a4LwJ8D04IBbMm16";
	const char * file = "/api/elevation?key=h0edaS8ftgo30a4LwJ8D04IBbMm16-&points=";
	if (client.connectClient() != 0) printf("Error connecting client!\n");
	std::stringstream ss;
	int loops = num / 10 + 1;
	for (int i = 0; i < loops; ++i) { //max 10 points per request
		ss.str("");
		ss << file;
		bool send = false;
		for (int j = 0; j < 10; ++j) {
			if (i * 10 + j >= num) break;
			ss << "(" << points[i * 10 + j].z << "," << points[i * 10 + j].x << ")";
			send = true;
			if (i * 10 + j + 1 < num) ss << ",";
			else break;
		}
		if (!send) break;
		request.file = ss.str();
		request.composeRequest();
		printf("Request: %s\n", request.data.c_str());
		client.sendMessage(request);
		HttpFrame response;
		client.getMessage(response);
		printf("Response: %s\n", response.content.c_str());
		auto searchStart = response.content.cbegin();
		std::smatch match;
		int j = 0;
		while (std::regex_search(searchStart, response.content.cend(), match, std::regex("\"elevation\":"))) {
			size_t pos = match.position() + 12;
			std::string num = response.content.substr(pos + (searchStart - response.content.cbegin()), response.content.find('}', pos + (searchStart - response.content.cbegin())) - (pos + (searchStart - response.content.cbegin())));
			float h = std::stof(num);
			points[i * 10 + j].y = h;
			searchStart = match.suffix().first;
			++j;
		}
	}

}

std::string img::getImage(coordinate topLeft, coordinate btmRight)
{
	const char * app_id = "eue3ZmJeRloe49N4jGod";
	const char * app_code = "7GI8j3yr8Mn3vr_RvnHOLA";
	const char * path = "/mia/1.6/mapview";
	HttpsClient client("image.maps.api.here.com");
	if (client.connectClient() != 0) printf("Failed to connect to HERE\n");
	HttpFrame req, resp;
	req.protocol = "GET";
	req.headers["Host"] = "image.maps.api.here.com";
	req.headers["Accept"] = "image/*";
	char gdata[500];
	sprintf(gdata, "%s?app_id=%s&app_code=%s&bbox=%f,%f,%f,%f,%f,%f,%f,%f", path, app_id, app_code,
		topLeft.lat, topLeft.lon, topLeft.lat, btmRight.lon, btmRight.lat, topLeft.lon, btmRight.lat, btmRight.lon);
	req.file = gdata;
	req.composeRequest();
	client.sendMessage(req);
	std::stringstream data;
	client.getMessage(resp);
	data.write(resp.content.c_str(), resp.content.size());
	int contentLength = std::stoi(resp.headers["Content-Length"]);

	while (data.str().size() < contentLength) {
		client.getMessage(resp);
		data.write(resp.data.c_str(), resp.data.size());
	}
	return data.str();

}

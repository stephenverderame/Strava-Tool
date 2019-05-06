#define _CRT_SECURE_NO_WARNINGS
#include "Strava.h"
#include "Parser.h"
#include <fstream>
#include <iostream>
#include "Polyline.h"
float map(float minx, float maxx, float x, float miny, float maxy);
int main() {
	Winsock ctx;
	std::vector<height::h_coord> ps = { {41, -78}, {42, -78}, {41, -79}, {40, -77}, 
										{39, -75}, {40, -70}, {40, -60}, {30, -65}, 
										{40, -75}, {40.5, -70.5}, {42, -72} };
	height::getHeights(ps.data(), ps.size());
	for (auto vec : ps) {
		printf("(%f, %f, %f)\n", vec.x, vec.y, vec.z);
	}
	getchar();
	Strava strava;
	std::string activities = strava.getActivitiesList();
/*	std::ifstream f("testJson.json");
	std::string activities((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
	f.close();
	printf("%s\n", activities.c_str());
	std::ofstream o("test.txt");
	o << activities;
	o.close();*/
	activities = Util::removeAll(activities, { '\n', '\r', '\t' });
	activities = activities.substr(activities.find("[{"));
//	activities = std::regex_replace(activities, std::regex("\"summary_polyline\":\"[^\",:]+\","), "");
	JSONParser parser;
	parser.parse(activities);
//	parser.operator<<(std::cout);
	const token_t * n = parser.rootNode();
	for (auto c : n->children)
		printf("%s (%s) occured at %s\n", c->search("name")->value.c_str(), c->search("id")->value.c_str(), c->search("start_date_local")->value.c_str());
	std::string polyline = strava.getActivityPolyline(n->children[1]->search("id")->value);
	auto points = Polyline::decode(polyline);
	float left = 300, right = -300, btm = 300, top = -300;
	for (coordinate & coord : points) {
		if (coord.lat < btm) btm = coord.lat;
		if (coord.lat > top) top = coord.lat;
		if (coord.lon > right) right = coord.lon;
		if (coord.lon < left) left = coord.lon;
	}
	printf("Left: %f\nRight: %f\nTop: %f\nBottom %f\n", left, right, top, btm);
	height::h_coord plane[20][20];
	for (int i = 0; i < 20; ++i) {
		for (int j = 0; j < 20; ++j) {
			plane[i][j].x = map(0, 20, j, left, right);
			plane[i][j].y = map(0, 20, i, btm, top);
		}
	}
	height::getHeights(&plane[0][0], sizeof(plane) / sizeof(height::h_coord));
	getchar();
	return 0;
}

float map(float minx, float maxx, float x, float miny, float maxy)
{
	return (maxy - miny) * (x - minx) / (maxx - minx) + miny;
}

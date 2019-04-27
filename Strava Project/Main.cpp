#define _CRT_SECURE_NO_WARNINGS
#include "Strava.h"
#include "Parser.h"
#include <fstream>
#include <iostream>
#include "Polyline.h"
int main() {
	Winsock ctx;
	Strava strava;
	std::string activities = strava.getActivitiesList();
/*	std::ifstream f("testJson.json");
	std::string activities((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
	f.close();
/*	printf("%s\n", activities.c_str());
	std::ofstream o("test.txt");
	o << activities;
	o.close();*/
	activities = Util::removeAll(activities, { '\n', '\r', ' ', '\t' });
	activities = activities.substr(activities.find("[{"));
//	activities = std::regex_replace(activities, std::regex("\"summary_polyline\":\"[^\",:]+\","), "");
	JSONParser parser;
	parser.parse(activities);
//	parser.operator<<(std::cout);
	std::ofstream out;
	out.open("test.json");
	parser.operator<<(out);
	out.close();
	const token_t * n = parser.rootNode();
	for (auto c : n->children)
		printf("%s (%s) occured at %s\n", c->search("name")->value.c_str(), c->search("id")->value.c_str(), c->search("start_date_local")->value.c_str());
	std::string polyline = strava.getActivityPolyline(n->children[0]->search("id")->value);
	out.open("testpoly.txt");
	out << polyline;
	out.close();
	printf("%s\n", polyline.c_str());
	getchar();
	return 0;
}
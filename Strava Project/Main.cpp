#define _CRT_SECURE_NO_WARNINGS
#include "Strava.h"
#include "Parser.h"
#include <fstream>
#include <iostream>
#include "Polyline.h"
#include <Window.h>
#include <UserInput.h>
#include <Engine.h>
#include "Terrain.h"
#include <Shape.h>
#define min(a, b) ((a) < (b) ? (a) : (b))
float map(float minx, float maxx, float x, float miny, float maxy);
int main() {
	Winsock ctx;
	Strava strava;
	if (!strava.isInit()) return -1;
	std::string activities = strava.getActivitiesList();
	activities = Util::removeAll(activities, { '\n', '\r', '\t' });
	activities = activities.substr(activities.find("[{"));
//	activities = std::regex_replace(activities, std::regex("\"summary_polyline\":\"[^\",:]+\","), "");
	JSONParser parser;
	parser.parse(activities);
//	parser.operator<<(std::cout);
	const token_t * n = parser.rootNode();
	const token_t * chosen = nullptr;
	for (auto c : n->children) {
		printf("%s (%s) occured at %s\n", c->search("name")->value.c_str(), c->search("id")->value.c_str(), c->search("start_date_local")->value.c_str());
		if (c->search("name")->value.find("Rapha") != std::string::npos) chosen = c;
	}
	std::string polyline = strava.getActivityPolyline(chosen->search("id")->value);
	std::ofstream out("testPolyline.txt");
	out << polyline << "\r\n\r\n";
	auto points = Polyline::decode(polyline);
	std::vector<glm::vec3> route;
	float left = 300, right = -300, btm = 300, top = -300;
	for (coordinate & coord : points) {
		if (coord.lat < btm) btm = coord.lat;
		if (coord.lat > top) top = coord.lat;
		if (coord.lon > right) right = coord.lon;
		if (coord.lon < left) left = coord.lon;
		out << "(" << coord.lat << ", " << coord.lon << ")\n";
		route.emplace_back(coord.lon, 0, coord.lat);
	}
	out.close();
	printf("Left: %f\nRight: %f\nTop: %f\nBottom %f\n", left, right, top, btm);
	constexpr size_t pts = 10;
	glm::vec3 plane[pts][pts];
	for (int i = 0; i < pts; ++i) {
		for (int j = 0; j < pts; ++j) {
			plane[i][j].x = map(0, pts, j, left, right);
			plane[i][j].z = map(0, pts, i, btm, top);
		}
	}
/*	std::vector<glm::vec3> route;
	for (int i = 0; i < points.size(); ++i)
		route.emplace_back(points[i].lon * 500, 0, points[i].lat * 500);*/
	height::getHeights(&plane[0][0], sizeof(plane) / sizeof(glm::vec3));
	for (size_t i = 0; i < pts * pts; ++i)
		plane[0][i] *= glm::vec3(250, 0.25, 250);
//	height::getHeights(route.data(), route.size());
	Window wind(800, 800, "Strava Demo");
	wind.antialiasing(8);
	wind.createWindow();
	glEnable(GL_PROGRAM_POINT_SIZE);
	glPointSize(10.f);
	wind.captureCursor();
	Camera cam(plane[pts / 2][pts / 2]);
//	Camera cam(route[1]);
	UserInput ui;
	wind.attatch(&ui);
	ui.setCamera(&cam);
	ui.standardMovement(true);
	ui.addKeyBind(keys::esc, [&wind](actions a, keys k) {
		if (a == actions::press) wind.releaseCursor();
	});
	Engine engine;
	wind.attatch(&engine);
	engine.perspective(glm::radians(45.f), 1.f, 0.1f, 500.f);
	Scene scene;
	Terrain terrain(&plane[0][0], pts);
//	Terrain terrain(route.data(), sqrt(route.size()));
	terrain.setColor({ 1.0, 0, 0 });
	scene.ADD_OBJ(terrain);
	Skybox sky("C:\\Users\\stephen\\Downloads\\cloudy\\bluecloud.jpg");
	scene.addSkybox(&sky);
	dirLight light(plane[pts / 2][pts / 2] + glm::vec3(0, 100, 0), glm::vec3(0.8), 0.3, 0.01);
	scene.addLight(&light);
	GAME_LOOP(wind,
		engine.view(cam);
		engine.render(scene);
	)
	return 0;
}

float map(float minx, float maxx, float x, float miny, float maxy)
{
	return (maxy - miny) * (x - minx) / (maxx - minx) + miny;
}

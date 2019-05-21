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
#include "res.h"
#define min(a, b) ((a) < (b) ? (a) : (b))
const int WindowWidth = 800, WindowHeight = 800;
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
	//	std::ofstream out("testPolyline.txt");
	//	out << polyline << "\r\n\r\n";
	auto points = Polyline::decode(polyline);
	float left = 300, right = -300, btm = 300, top = -300;
	for (coordinate & coord : points) {
		if (coord.lat < btm) btm = coord.lat;
		if (coord.lat > top) top = coord.lat;
		if (coord.lon > right) right = coord.lon;
		if (coord.lon < left) left = coord.lon;
		//		out << "(" << coord.lat << ", " << coord.lon << ")\n";
	}
	//	out.close();
	printf("Left: %f\nRight: %f\nTop: %f\nBottom %f\n", left, right, top, btm);
	constexpr size_t pts = 10;
	glm::vec3 plane[pts][pts];
	for (int i = 0; i < pts; ++i) {
		for (int j = 0; j < pts; ++j) {
			plane[i][j].x = map(0, pts, j, left, right);
			plane[i][j].z = map(0, pts, i, btm, top);
		}
	}
	const float scale = 250;
	height::getHeights(&plane[0][0], sizeof(plane) / sizeof(glm::vec3));
	for (size_t i = 0; i < pts * pts; ++i)
		plane[0][i] *= glm::vec3(scale, 0.05, scale);
	std::vector<glm::vec3> route;
	for (size_t i = 0; i < points.size(); ++i) {
		route.emplace_back(points[i].lon * scale, 0, points[i].lat * scale);
	}
	//	height::getHeights(route.data(), route.size());
	Window wind(WindowWidth, WindowHeight, "Strava Demo");
	wind.antialiasing(8);
	wind.createWindow();
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
	engine.perspective(glm::radians(45.f), (float)WindowWidth / WindowHeight, 0.1f, 500.f);
	Scene scene;
	//	Terrain terrain(route.data(), sqrt(route.size()));
	Terrain terrain(&plane[0][0], pts);
//	terrain.setColor({ 1.0, 0, 0 });
	scene.ADD_OBJ(terrain);
	//	scene.ADD_OBJ(trail);
	Skybox sky("C:\\Users\\stephen\\Downloads\\cloudy\\bluecloud.jpg");
	scene.addSkybox(&sky);
	dirLight light(plane[pts / 2][pts / 2] + glm::vec3(0, 100, 0), glm::vec3(0.8), 0.3, 0.01);
	scene.addLight(&light);

	std::unique_ptr<Shader> trailShader = std::make_unique<Shader>(CUSTOM_SHADER, FBO_VERTEX, FBO_FRAGMENT);
	for (size_t i = 0; i < route.size(); ++i) {
		route[i].x = map(left * scale, right * scale, route[i].x, 0, Trail::textureWidth);
		route[i].z = map(btm * scale, top * scale, route[i].z, 0, Trail::textureHeight);
		route[i].y = 0;
	}
	Trail trail(route.data(), route.size());
	trail.setColor({ 0.0, 1.0, 0.0 });
	trail.setBgColor({ 1.0, 0.0, 0.0, 1.0 });
	trailShader->use();
	trailShader->setMat4("projection", glm::ortho(0.f, (float)Trail::textureWidth, 0.f, (float)Trail::textureHeight, -1.f, 1.f));
//	glEnable(GL_LINE_WIDTH);
//	glLineWidth(100.f);
	GAME_LOOP(wind,
		trailShader->use();
		trail.draw(trailShader.get());
		terrain.setTexture(trail.getTexture());
		engine.view(cam);
		engine.render(scene);
	)
	return 0;
}



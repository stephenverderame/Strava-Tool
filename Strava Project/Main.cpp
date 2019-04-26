#define _CRT_SECURE_NO_WARNINGS
#include "Strava.h"
#include "Parser.h"
#include <fstream>
#include <iostream>
int main() {
	Winsock ctx;
/*	HttpListener http(8060);
	FD read;
	read.add(&http);
	read.clear();
	HttpsClient fetch((char*)"www.strava.com");
	HttpFrame requestFrame;
	requestFrame.protocol = HTTP_GET;
	requestFrame.file = "/oauth/authorize?client_id=34668&scope=read&redirect_uri=http://127.0.0.1:8060&response_type=code";
	requestFrame.headers["Host"] = "www.strava.com";
	requestFrame.headers["Accept-Encoding"] = "identity,chunked;q=0.9";
	requestFrame.headers["Accept-Language"] = "en-US,en;q=0.9";
	requestFrame.headers["Accept"] = "text/html,application/xhtml+xml,application/xml";
	requestFrame.headers["Connection"] = "keep-alive";
	requestFrame.headers["Upgrade-Insecure-Requests"] = "1";
	requestFrame.headers["User-Agent"] = "Mozilla/5.0 (Linux; Android 5.0; SM-G900P Build/LRX21T) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/73.0.3683.103 Mobile Safari/537.36";
	requestFrame.composeRequest();
	if(fetch.connectClient() != 0) printf("Connection to strava failed!");
	printf("%s\n", requestFrame.data.c_str());
	fetch.sendMessage(requestFrame);
	HttpFrame respFrame;
	fetch.getMessage(respFrame);
	printf("%s\n\n", respFrame.data.c_str());

	requestFrame.file = "/login";
	size_t sessionCookieLocation = respFrame.data.find("_strava4_session=");
	requestFrame.headers["Cookie"] = respFrame.data.substr(sessionCookieLocation, respFrame.data.find(" ", sessionCookieLocation) - sessionCookieLocation);
	requestFrame.composeRequest();
	fetch.sendMessage(requestFrame);
	fetch.getMessage(respFrame);
	printf("%s\n\n", respFrame.data.c_str());

	requestFrame.file = "/session";
	requestFrame.content = "utf8=%E2%9C%93&authenticity_token=YoL%2FupqJKWQAcovTWfRwQz4aZ7ZxX1szuEN4vTQP1xYTKPyLYcqw85m5IW2gPo05BXQGVl7ovGZ16lp7%2BzUEvw%3D%3D&plan=&email=stephenverderame%40gmail.com&password=Biker1080\r\n";
	requestFrame.protocol = HTTP_POST;
	requestFrame.headers["Origin"] = "https://www.strava.com";
	requestFrame.headers["Content-Type"] = "application/x-www-form-urlencoded";
	requestFrame.headers["Content-Length"] = std::to_string(requestFrame.content.length() - 2);
	requestFrame.headers["Referer"] = "https://www.strava.com/login";
	requestFrame.composeRequest();
	printf("%s\n", requestFrame.data.c_str());
	fetch.sendMessage(requestFrame);
	fetch.getMessage(respFrame);
	printf("%s\n\n\n", respFrame.data.c_str());

//	FD::wait(&read);
//	printf("Got request to server!\n");
	int error;
	HtmlFile testFile("C:\\Users\\stephen\\Desktop\\main.html");
	HttpClient client = http.accept(error);
	printf("Accept Error: %d\n", error);
	HttpFrame frame;
	client.getMessage_b(frame);
	printf("%s\n", frame.data.c_str());
	client.sendHtmlFile(testFile);*/
	Strava strava;
	std::string activities = strava.getActivitiesList();
	printf("%s\n", activities.c_str());
	std::ofstream o("test.txt");
	o << activities;
	o.close();
	activities = activities.substr(activities.find("[{"));
	activities = Util::removeAll(activities, { '\n', '\r', ' '});
	activities = std::regex_replace(activities, std::regex("\"summary_polyline\":\"[^\",:]+\","), "");
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
	getchar();
	return 0;
}
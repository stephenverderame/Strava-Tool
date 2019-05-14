#define _CRT_SECURE_NO_WARNINGS
#include "Strava.h"
#include <Windows.h>
#include "Parser.h"
#include <fstream>
#include <regex>

const int Strava::client_id = 34668;
const char * const Strava::client_secret = "0a01bd0adee247b04f2605a7d78ffc5f11a9ed93";


Strava::Strava()
{
	HttpListener authenticator(8060);
	ShellExecute(NULL, 0, "https://www.strava.com/oauth/authorize?client_id=34668&scope=activity:read_all&redirect_uri=http://127.0.0.1:8060&response_type=code", 0, 0, SW_SHOW);
	int error;
	HttpClient authClient = authenticator.accept(error);
	if (error != 0) printf("Accept error: %d\n", error);
	HttpFrame request;
	authClient.getMessage_b(request);

//	printf("%s\n\n", request.data.c_str());
	size_t codeLocation = request.data.find("code=");
	bool fail;
	if (fail = codeLocation == std::string::npos) printf("Could not find code!\n");
	else authCode = request.data.substr(codeLocation + 5, request.data.find('&', codeLocation) - (codeLocation + 5)).c_str();
	
	HtmlFile response;
	std::ifstream in("Login.html");
	std::stringstream ss;
	std::string line;
	while (std::getline(in, line)) ss << line;
	std::string file = std::regex_replace(ss.str(), std::regex("\\<\\?php.*\\?\\>"), fail ? "Failure" : "Success");
	response.cpyFromMem(file.c_str());
	authClient.sendHtmlFile(response);
	if (fail) return;

	HttpsClient authStage2("www.strava.com");
	HttpFrame tokenRequest;
	tokenRequest.protocol = "POST";
	tokenRequest.file = "/oauth/token";
	char content[500];
	//	sprintf(content, "client_id=%d&client_secret=%s&code=%s&grant_type=authorization_code\r\n", client_id, urlencode(client_secret).get(), urlencode(authCode.c_str()).get());
	sprintf(content, "{\"client_id\": \"%d\",\"client_secret\": \"%s\",\"code\": \"%s\",\"grant_type\": \"authorization_code\"}\r\n", client_id, client_secret, authCode.c_str());
	tokenRequest.content = content;// gzip::compress(content, strlen(content));
	tokenRequest.headers["Host"] = "www.strava.com";
	tokenRequest.headers["Content-Type"] = "application/json";
	//	tokenRequest.headers["Content-Type"] = "application/x-www-form-urlencoded";
	tokenRequest.headers["Content-Length"] = std::to_string(tokenRequest.content.size());
	//	tokenRequest.headers["Accept"] = "text/html,application/json,application/xml,application/xhtml+xml";
	tokenRequest.headers["Accept"] = "application/json, */*";
	tokenRequest.headers["Connection"] = "keep-alive";
	//	tokenRequest.headers["Accept-Encoding"] = "gzip";
	//	tokenRequest.headers["Content-Encoding"] = "gzip";
	tokenRequest.headers["User-Agent"] = "HTTPie/1.0.2";
	tokenRequest.composeRequest();
//	printf("%s\n", tokenRequest.data.c_str());
	if (0 != authStage2.connectClient()) printf("Failed to connect to strava!\n");
	error = authStage2.sendMessage(tokenRequest);
//	printf("Send code: %d\n", error);
	HttpFrame responseFrame;
	error = authStage2.getMessage(responseFrame);
	bool ok = responseFrame.data.find("200") != std::string::npos;
/*	while (ok && responseFrame.data.find("access_token") == std::string::npos) {
		authStage2.getMessage(responseFrame);
	}*/
	if (ok) {
//		printf("%s\n", responseFrame.data.c_str());
		size_t tokenLocal = responseFrame.data.find("\"access_token\":");
		accessToken = responseFrame.data.substr(tokenLocal + 16, responseFrame.data.find(',', tokenLocal) - (tokenLocal + 16) - 1);
		printf("\n\nToken: %s\n", accessToken.c_str());
	}
}


Strava::~Strava()
{
}

std::string Strava::getActivitiesList(int page, int perPage)
{
	std::stringstream buffer;
	HttpsClient client("www.strava.com");
	HttpFrame request;
	request.protocol = "GET";
	request.file = "/api/v3/athlete/activities?page=" + std::to_string(page) + "&per_page=" + std::to_string(perPage);
	request.headers["Host"] = "www.strava.com";
	request.headers["Accept"] = "application/json, */*";
	request.headers["Connection"] = "keep-alive";
	request.headers["Authorization"] = "Bearer " + accessToken;
	client.connectClient();
	request.composeRequest();
	client.sendMessage(request);
	HttpFrame response;
	client.getMessage(response);
	bool ok = response.data.find("OK") != std::string::npos;
	buffer << response.data;
/*	while (ok && buffer.str().find("\r\n0\r\n") == std::string::npos) { //until it finds end of chunk
		client.getMessage(response);
		buffer << response.data;
	}*/
	return buffer.str();

}

std::string Strava::getActivityPolyline(const std::string & id)
{
	HttpsClient client("www.strava.com");
	HttpFrame request;
	request.protocol = "GET";
	request.file = "/api/v3/activities/" + id + "?include_all_efforts=false";
	request.headers["Host"] = "www.strava.com";
	request.headers["Accept"] = "application/json, */*";
	request.headers["Connection"] = "keep-alive";
	request.headers["Authorization"] = "Bearer " + accessToken;
	client.connectClient();
	request.composeRequest();
	client.sendMessage(request);
	HttpFrame response;
	client.getMessage(response);
	if (response.data.find("200 OK") != std::string::npos) {
		JSONParser parser;
		std::ofstream test("test.txt");
		test << response.data;
		test.close();
		std::string data = response.data.substr(response.data.find("{"));
		parser.parse(data);
		return std::regex_replace(parser.search("polyline", "map"), std::regex("\\\\\\\\"), "\\"); // matches with "\\" replaces with "\"
	}
	return "";
}

bool Strava::isInit()
{
	return authCode.length() > 0 && accessToken.length() > 0;
}

std::unique_ptr<char[]> Strava::urlencode(const char * c) {
	std::unique_ptr<char[]> ptr = std::make_unique<char[]>(strlen(c) * 3);
	for (int i = 0, j = 0; i < strlen(c); ++i, ++j) {
		if (!isalnum(c[i])) {
			*(ptr.get() + j++) = '%';
			sprintf(ptr.get() + j, "%02X", c[i]);
			j++;
		}
		else {
			*(ptr.get() + j) = c[i];
		}
	}
	return ptr;
}
std::unique_ptr<char[]> Strava::urldecode(const char * c) {
	std::unique_ptr<char[]> ptr = std::make_unique<char[]>(strlen(c));
	for (int i = 0, j = 0; i < strlen(c); ++i, ++j) {
		if (i < strlen(c) - 2 && c[i] == '%') {
			char num[3];
			memcpy(num, c + i + 1, 2);
			num[2] = '\n';
			*(ptr.get() + j) = (char)strtol(num, NULL, 16);
			i += 2;
		}
		else *(ptr.get() + j) = c[i];
	}
	return ptr;
}

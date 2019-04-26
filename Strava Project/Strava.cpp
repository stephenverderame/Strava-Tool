#define _CRT_SECURE_NO_WARNINGS
#include "Strava.h"
#include <Windows.h>
#include <gzip/compress.hpp>
#include <gzip/decompress.hpp>

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
	HtmlFile response("Login.html");
	authClient.sendHtmlFile(response);
//	printf("%s\n\n", request.data.c_str());
	size_t codeLocation = request.data.find("code=");
	if (codeLocation == std::string::npos) printf("Could not find code!\n");
	else authCode = request.data.substr(codeLocation + 5, request.data.find('&', codeLocation) - (codeLocation + 5)).c_str();
//	printf("Auth Code:%s\n", authCode.c_str());

	HttpsClient authStage2((char*)"www.strava.com");
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
	while (ok && responseFrame.data.find("access_token") == std::string::npos) {
		authStage2.getMessage(responseFrame);
	}
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

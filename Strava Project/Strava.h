#pragma once
#include <Https.h>
#include <memory>
class Strava
{
public:
	static const int client_id;
	static const char * const client_secret;
private:
	std::string authCode;
	std::string accessToken;
public:
	Strava();
	~Strava();
	std::string getActivitiesList(int page = 1, int perPage = 30);
public:
	static std::unique_ptr<char[]> urlencode(const char * c);
	static std::unique_ptr<char[]> urldecode(const char * c);
};


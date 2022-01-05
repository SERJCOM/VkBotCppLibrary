#define CURL_STATICLIB
#include <iostream>
#include <curl/curl.h> 
#include <commapi.h>
#include <map>
#include <typeinfo>
#include <json/json.h>
#include "commands.h"
#include <string>

Bot::Bot(std::string tocken) {
	this->key_access = tocken;

	curl = curl_easy_init();
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl_easy_setopt(curl, CURLOPT_URL, ("https://api.vk.com/method/groups.getLongPollServer?group_id=200085488&v=5.131&access_token="+ tocken).c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);
	res = curl_easy_perform(curl);

	bool parsingSuccessful = reader.parse(content, root);
	std::string request = "response";
	bool encoding = root.isMember(request);
	if (encoding) { std::cout << "true" << std::endl; }
	else { std::cout << "false" << std::endl; }

	key = root["response"]["key"].asString();
	server = root["response"]["server"].asString();
	ts = root["response"]["ts"].asString();
}

Bot::~Bot() {
	curl_easy_cleanup(curl);
}

size_t Bot::write_data(char* ptr, size_t size, size_t nmemb, std::string* data)
{
	if (data)
	{
		data->clear();
		data->append(ptr, size * nmemb);
		return size * nmemb;
	}
	else { return 0; } // будет ошибка
}

void Bot::update() {
	curl_easy_setopt(curl, CURLOPT_URL, (server + "?act=a_check&key=" + key + "&ts=" + ts + "&wait1").c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content_js);
	res = curl_easy_perform(curl);
	bool parsingSuccessful = reader.parse(content_js, root);
	if (!parsingSuccessful){std::cout << "Failed to parse configuration\n" << reader.getFormattedErrorMessages();}
	message = root["updates"][0]["object"]["text"].asString();
	user_id = root["updates"][0]["object"]["from_id"].asString();
	ts = root["ts"].asString();
	type = root["updates"][0]["type"].asString();
}

void Bot::send(std::string answer, std::string user_id) {
	std::string s = answer;
	std::string::size_type n;
	while (true) {
		n = s.find(" ");
		if (n == std::string::npos) {
			break;
		}
		s = s.replace(n, 1, 1, '+');
	}
	answer = s;
	curl_easy_setopt(curl, CURLOPT_URL, (u8"https://api.vk.com/method/messages.send?user_id=" + user_id + "&message=" + answer + "&random_id=0&v=5.131&access_token=" + key_access).c_str());
	res = curl_easy_perform(curl);
	if (res != CURLE_OK)
	{
		std::cout << "Error!" << std::endl;
		std::cout << curl_easy_strerror(res) << std::endl;
	}
}


int main()
{
	std::string access_key ;

	Bot bot(access_key);
	while (true) {
		bot.update();
		if (bot.type == "message_new") {
			bot.send(bot.message, bot.user_id);
		}
	}
	return 0;
}

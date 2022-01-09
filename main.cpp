#define CURL_STATICLIB
#include <iostream>
#include <curl/curl.h> 
#include <commapi.h>
#include <map>
#include <typeinfo>
#include <json/json.h>
#include "commands.h"
#include <string>
#include <fstream>
#include <clocale>
#include <Windows.h>

Bot::Bot(std::string tocken, std::string id_my_group) {
	this->key_access = tocken;

	curl = curl_easy_init();
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl_easy_setopt(curl, CURLOPT_URL, ("https://api.vk.com/method/groups.getLongPollServer?group_id="+ id_my_group+"&v=5.131&access_token="+ tocken).c_str());
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
	if (res != CURLE_OK)
	{
		std::cout << "Error!" << std::endl;
		std::cout << curl_easy_strerror(res) << std::endl;
	}
	bool parsingSuccessful = reader.parse(content_js, root);
	if (!parsingSuccessful) { 
		std::cout << "Failed to parse configuration\n" << reader.getFormattedErrorMessages();
		std::cout << root << std::endl;
	}

	message = root["updates"][0]["object"]["text"].asString();
	peer_id = root["updates"][0]["object"]["peer_id"].asString();
	ts = root["ts"].asString();
	group_id = root["updates"][0]["object"]["from_id"].asInt();
	type = root["updates"][0]["type"].asString();
	if (type == "message_new" && group_id > 0) {
		std::cout << root << std::endl;
		//std::cout << "СООБЩЕНИЕ: " << root["updates"][0]["object"]["text"].toStyledString()<< "\n" << "PEER_ID: " << peer_id << "\n" << "TS: " << ts <<  std::endl;
	}
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
	curl_easy_setopt(curl, CURLOPT_URL, (u8"https://api.vk.com/method/messages.send?peer_id=" + user_id + "&message=" + answer + "&random_id=0&v=5.131&access_token=" + key_access).c_str());
	res = curl_easy_perform(curl);
	if (res != CURLE_OK)
	{
		std::cout << "Error!" << std::endl;
		std::cout << curl_easy_strerror(res) << std::endl;
	}

}

void Bot::write(std::string message) {
	outf.open("dict/" + peer_id + ".txt", std::ios::app);
	outf << message << std::endl;
	outf.close();
}

std::string Bot::find_message(std::string message) {
	inf.open("dict/" + peer_id +".txt");
	int count = 0;
	std::string::size_type n;
	std::string old_line = "старая строка";
	std::string line;
	bool flag = false;
	std::string return_message;
	while (inf)
	{
		count++;
		getline(inf, line);
		n = line.find(message);
		
		if (n != std::string::npos) {
			inf.close();
			return_message = old_line;
			flag = true;
		}
		old_line = line;
	}
	inf.close();
	count_lines = count;
	if (flag) {
		return return_message;
	}
	return "None";
}

int getRandomNumber(int min, int max)
{
	static const double fraction = 1.0 / (static_cast<double>(RAND_MAX) + 1.0);
	return static_cast<int>(rand() * fraction * (max - min + 1) + min);
}

std::string Bot::random_message(int count) {
	int randnum = getRandomNumber(1, 4);
	std::string message;
	for (int i = 0; i < randnum; i++) {
		inf.open("dict/" + peer_id + ".txt");
		for (int j = 1; j < count; j++) {
			std::string n;
			getline(inf, n);
			if (j == getRandomNumber(1, count)) {
				message = message + n + " ";
			}
		}
		inf.close();
	}
	return message;
}

void Bot::conf_init() {


}

int main()
{
	srand(static_cast<unsigned int>(time(0)));
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	std::string access_key = "10a3851d320f0e3cf3fc7b988363fab485a91b975d3673012291adcc9773d8a2d50385febbb9217609ef8";
	std::string id_my_group = "200638400";
	Bot bot(access_key, id_my_group);
	
	while (true) {
		bot.update();
		if (bot.type == "message_new" && bot.group_id > 0) {
			std::cout << "=====NEW MESSAGE=======\n " << std::endl;
			std::string message = bot.find_message(bot.message);
			if (message == "None") {
				std::cout << "random message\n";
				message = bot.random_message(bot.count_lines);
			}
			bot.write((bot.message).c_str());
			bot.send(message.c_str(), bot.peer_id);
		}
	}
	return 0;
}

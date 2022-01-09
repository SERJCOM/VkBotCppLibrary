#pragma once
#define CURL_STATICLIB
#include<string>
#include <iostream>
#include <curl/curl.h> 
#include <json/json.h>
#include <string>
#include <fstream>


class Bot {
public:
	Bot(std::string key_access, std::string id_my_group);
	~Bot();
	void update();
	void send(std::string answer, std::string user_id);
	void write(std::string message);
	void conf_init();
	std::string find_message(std::string message);
	std::string random_message(int count);

	std::string message;
	std::string chat_id;
	std::string user_id;
	std::string type;
	int group_id;
	std::string peer_id;
	int count_lines;
	
private:
	static size_t write_data(char* ptr, size_t size, size_t nmemb, std::string* data);
	std::string key_access;
	Json::Value root;
	Json::Value json();
	Json::Reader reader;
	CURL* curl;
	CURLcode res;
	std::string key;
	std::string server;
	std::string ts;
	std::string content;
	std::string content_js;
	std::ofstream outf;
	std::ifstream inf;
	
};

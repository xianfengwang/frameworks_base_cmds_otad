/*
 * HttpClient.cpp
 *
 *  Created on: 2014-5-21
 *      Author: manson
 */
#include "net/HttpClient.h"
#include "utils/utils.h"
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <map>
#include "curl/curl.h"

using namespace std;

size_t saveToBuffer(char *data, size_t size, size_t nmemb,
		std::string *writerData);

HttpClient::HttpClient() {
}

HttpClient::~HttpClient() {
}

string HttpClient::GetRequest(string url) {
	CURL *curl;
	string buffer;
	char errbuf[CURL_ERROR_SIZE] = { 0 };
	long responseCode = 0;
	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, HTTP_REQUEST_TIME_OUT);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void * )&buffer);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, saveToBuffer);
		CURLcode ret = curl_easy_perform(curl);
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
		curl_easy_cleanup(curl);
		if (CURLE_OK != ret || responseCode != 200) {
			LOGD("err info = %s", errbuf);
			LOGD("response code = %ld", responseCode);
		}
	}
	return buffer;
}

string HttpClient::PostRequest(string url, string data) {
	CURL *curl;
	string buffer;
	char errbuf[CURL_ERROR_SIZE] = { 0 };
	long responseCode = 0;
	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, 1);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, HTTP_REQUEST_TIME_OUT);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void * )&buffer);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, saveToBuffer);
		CURLcode ret = curl_easy_perform(curl);
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
		curl_easy_cleanup(curl);
		if (CURLE_OK != ret || responseCode != 200) {
			LOGD("err info = %s", errbuf);
			LOGD("response code = %ld", responseCode);
		}
	}
	return buffer;
}

size_t saveToBuffer(char *data, size_t size, size_t nmemb, string *writerData) {
	size_t sizes = size * nmemb;
	if (NULL == data) {
		return 0;
	}
	writerData->append(data);
	return sizes;
}

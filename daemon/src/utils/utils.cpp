#include "utils/utils.h"
#include <string>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <vector>
#include <string.h>
#include <sstream>
#include "fs/md5.h"

using namespace std;

void CUSTLOG(const char * _Format, ...) {
	return;
}

string u_trimHttpPrefix(string url) {
	string trimed = url;
	if (trimed.find("http://") != string::npos) {
		return trimed.substr(strlen("http://"), trimed.length());
	}
	return url;
}

string u_trimSpace(string content) {
	string trimed = content.substr(content.find_first_not_of(' '),
			content.find_last_not_of(' ') + 1);
	return trimed;
}

string u_trimUrlGetName(string url) {
	string trimed = url.substr(url.find_last_of('/') + 1, url.size());
	return trimed;
}

/*
 * typically, prefixLength == len(prefix)+1
 */
string u_getValueFromProcFile(const string path, const string prefix,
		int prefixLength, string defaultValue) {
	string buffer;
	string value;
	unsigned int pos;
	ifstream ifs;
	ifs.open(path.c_str(), ios_base::in);
	while (getline(ifs, buffer, '\n')) {
		if (string::npos != (pos = buffer.find(prefix))) {
			value = buffer.substr(pos + prefixLength, buffer.size());
		}
	}
	ifs.close();
	return value.size() > 0 ? value : defaultValue;
}

/*
 *get the first line from proc file only
 */
string u_getValueFromProcFile(string path, string defaultValue) {
	string buffer;
	int pos;
	ifstream ifs;
	ifs.open(path.c_str(), ios_base::in);
	getline(ifs, buffer, '\n');
	if (buffer.length() > 0) {
		return buffer;
	}
	ifs.close();
	return defaultValue;
}

long u_getCurrentTime() {
	long timeTick = 0;
	time(&timeTick);
	return timeTick;
}

string u_getDeviceIp() {
	char hostName[256];
	string ipAddr;
	struct hostent *hent = gethostbyname(hostName);
	if (hent != NULL) {
		ipAddr = inet_ntoa(*(struct in_addr*) (hent->h_addr_list[0]));
	}
	return ipAddr;
}

vector<string> u_split(const string& s, const string& match) {
	vector<string> result;                 // return container for tokens
	string::size_type start = 0,           // starting position for searches
			skip = 1;            // positions to skip after a match
	find_t pfind = &string::find_first_of; // search algorithm for matches
	while (start != string::npos) {
		string::size_type end = (s.*pfind)(match, start);
		if (skip == 0)
			end = string::npos;
		string token = s.substr(start, end - start);
		if (!token.empty()) {
			result.push_back(token);
		}
		if ((start = end) != string::npos) {
			start += skip;
		}
	}
	return result;
}

void u_splitProperty(vector<string> property, string* name, string *value) {
	vector<string>::iterator it = property.begin();
	if (it != property.end()) {
		name->clear();
		name->append(*it);
	}
	it++;
	if (it != property.end()) {
		value->clear();
		value->append(*it);
	}
}

bool u_getJsonValueFromString(string jsonString, Json::Value &value) {
	Json::Reader reader;
	if (!reader.parse(jsonString, value)) {
		LOGD("bad json format : %s", jsonString.c_str());
		return false;
	}
	return true;
}

string u_int2string(int i) {
	stringstream ss;
	ss << i;
	return ss.str();
}

bool compareMd5(string path, string value) {
	bool res = false;

	MD5 *md5 = new MD5();
	md5->updateFile(path);
	string mStr = md5->toString();
	LOGD("compute file's md5 = %s", mStr.c_str());
	LOGD("standard md5 = %s", value.c_str());
	if (mStr == value) {
		res = true;
	}
	delete md5;

	return res;
}

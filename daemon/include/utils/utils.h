#ifndef _UTILS_H_
#define _UTILS_H_

#include <utils/Log.h>
#include <string>
#include <vector>
#include "json/json.h"

#define LOG_TAG "OTAD"
void CUSTLOG(const char * _Format, ...);
#define OTADLOG
#ifndef OTADLOG
#define LOGD  CUSTLOG
#define LOGI   CUSTLOG
#define LOGE  CUSTLOG
#else
#define LOGD  ALOGD
#define LOGI   ALOGI
#define LOGE  ALOGE
#endif

using std::string;

string u_trimUrlGetName(string url);
string u_trimHttpPrefix(string url);
string u_trimSpace(string content);
string u_getValueFromProcFile(const string path, const string prefix,
		int prefixLength, string defaultValue);
string u_getValueFromProcFile(string path, string defaultValue);
long u_getCurrentTime();
string u_getDeviceIp();

string u_int2string(int i);

typedef string::size_type (string::*find_t)(const string& delim,
		string::size_type offset) const;

std::vector<string> u_split(const string& s, const string& match);
void u_splitProperty(std::vector<string> property, string* name, string *value);
bool u_getJsonValueFromString(string jsonString, Json::Value &value);
bool compareMd5(string path, string md5);
#endif

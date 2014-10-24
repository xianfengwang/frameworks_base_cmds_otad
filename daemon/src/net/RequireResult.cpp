/*
 * RequireResult.cpp
 *
 *  Created on: 2014-5-27
 *      Author: manson
 */

#include <string>
#include "net/RequireResult.h"
#include "utils/utils.h"
#include <sstream>

using namespace std;

RequireResult::RequireResult(string result) {
	m_szResult = result;

	m_nStatus = 0;
	m_dFileSize = 0;
}

RequireResult::~RequireResult() {
}

bool RequireResult::Load() {
	if (m_szResult.size() == 0) {
		return false;
	}

	Json::Reader reader;
	if (!reader.parse(m_szResult, m_jsonResult)) {
		LOGD("require result parse failed : %s", m_szResult.c_str());
		return false;
	}

	m_nStatus = m_jsonResult[KEY_STATUS].asInt();
	m_szUrl = m_jsonResult[KEY_URL].asString();
	m_szFileName = u_trimUrlGetName(m_szUrl);
	m_szCode = m_jsonResult[KEY_API_VERSION_CODE].asString();
	m_szMd5 = m_jsonResult[KEY_MD5].asString();

	m_dFileSize = m_jsonResult[KEY_SIZE].asDouble();

	return true;
}

void RequireResult::Log() {
	LOGD("status = %d", m_nStatus);
	LOGD("url = %s", m_szUrl.c_str());
	LOGD("fileName = %s", m_szFileName.c_str());
	LOGD("code = %s", m_szCode.c_str());
	LOGD("md5 = %s", m_szMd5.c_str());
	LOGD("fileSize = %f", m_dFileSize);
}

string RequireResult::Dump2String() {
	return m_jsonResult.toStyledString();
}

int RequireResult::GetStatus() {
	return m_nStatus;
}

string RequireResult::GetUrl() {
	return m_szUrl;
}

string RequireResult::GetCode() {
	return m_szCode;
}

string RequireResult::GetMd5() {
	return m_szMd5;
}

string RequireResult::GetFileName() {
	return m_szFileName;
}

double RequireResult::GetFileSize() {
	return m_dFileSize;
}

/*
 * UploadParams.cpp
 *
 *  Created on: 2014-5-23
 *      Author: manson
 */

#include "net/RequestParams.h"
#include <string>

using namespace std;

RequestParams::RequestParams() {
	m_DeviceInfo = new DeviceInfo();
	m_DeviceInfo->LoadFromProperty();
}

RequestParams::~RequestParams() {
	delete m_DeviceInfo;
}

bool RequestParams::Load() {
	if (!m_DeviceInfo) {
		return false;
	}
	m_jsonParams[KEY_DEVICE_INFO] = m_DeviceInfo->GetJsonValue();
	m_jsonParams[KEY_API_VERSION_CODE] = API_VERSION_CODE;
	return true;
}

string RequestParams::Dump2String() {
	return m_jsonParams.toStyledString();
}

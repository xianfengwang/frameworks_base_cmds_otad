/*
 * RequestParams.h
 *
 *  Created on: 2014-5-23
 *      Author: manson
 */

#ifndef REQUESTPARAMS_H_
#define REQUESTPARAMS_H_

#include "platform/DeviceInfo.h"
#include "json/json.h"
#include <string>

#define KEY_DEVICE_INFO 		"device_info"
#define KEY_API_VERSION_CODE 	"code"
#define API_VERSION_CODE		"10000"

class RequestParams {
public:
	RequestParams();
	virtual ~RequestParams();

	bool Load();
	std::string Dump2String();

private:
	DeviceInfo *m_DeviceInfo;
	Json::Value m_jsonParams;
};

#endif /* REQUESTPARAMS_H_ */

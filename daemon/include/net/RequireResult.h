/*
 * RequireResult.h
 *
 *  Created on: 2014-5-27
 *      Author: manson
 */

#ifndef REQUIRERESULT_H_
#define REQUIRERESULT_H_

#include "json/json.h"
#include <string>

#define KEY_STATUS				"status"
#define KEY_URL					"url"
#define KEY_MD5					"md5"
#define KEY_SIZE				"size"
#define KEY_API_VERSION_CODE	"code"

class RequireResult {
public:
	RequireResult(std::string result);
	virtual ~RequireResult();

	bool Load();
	std::string Dump2String();
	void Log();

	int GetStatus();
	std::string GetUrl();
	std::string GetCode();
	std::string GetMd5();
	std::string GetFileName();
	double GetFileSize();

private:
	Json::Value m_jsonResult;
	std::string m_szResult;

	int m_nStatus;
	double m_dFileSize;
	std::string m_szUrl;
	std::string m_szMd5;
	std::string m_szFileName;
	std::string m_szCode;
};

#endif /* REQUIRERESULT_H_ */

/*
 * HttpClient.h
 *
 *  Created on: 2014-5-21
 *      Author: manson
 */

#ifndef HTTPCLIENT_H_
#define HTTPCLIENT_H_

#include <string>

#define HTTP_REQUEST_TIME_OUT 	30 //in second

class HttpClient {
public:
	HttpClient();
	virtual ~HttpClient();

	std::string PostRequest(std::string url, std::string data);
	std::string GetRequest(std::string url);
};

#endif /* HTTPCLIENT_H_ */

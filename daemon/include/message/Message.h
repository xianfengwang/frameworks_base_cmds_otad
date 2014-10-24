/*
 * Message.h
 *
 *  Created on: 2014-6-28
 *      Author: manson
 */

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <string>
#include "IMessage.h"

#define	CHECK_JSON_FORMAT		false

class Message: public IMessage {
protected:
	std::string m_szType; //["SYSTEM_STATUS", "OTA_UPDATE"]
	std::string m_szData; //custom data
	std::string m_szMessage; //join all elements into a string

	virtual void Init() = 0;

public:
	Message();
	virtual ~Message();

	std::string GetType();
	std::string GetData();
	std::string GetMessage();
	void JoinElements();
	void Log();
};

#endif /* MESSAGE_H_ */

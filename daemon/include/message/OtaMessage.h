/*
 * OtaMessage.h
 *
 *  Created on: 2014-6-7
 *      Author: manson
 */

#ifndef OTAMESSAGE_H_
#define OTAMESSAGE_H_

#include "Message.h"
#include <string>

#define RETRY_COUNT			5
#define RETRY_INTERVAL		10

class OtaMessage: public Message {
private:
	std::string m_szEvent;
	std::string m_szResult;

protected:
	void Init();

public:
	OtaMessage(std::string event, std::string result);
	virtual ~OtaMessage();
	bool CanResent();
};

#endif /* OTAMESSAGE_H_ */

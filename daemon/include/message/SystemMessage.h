/*
 * SystemMessage.h
 *
 *  Created on: 2014-6-28
 *      Author: manson
 */

#ifndef SYSTEMMESSAGE_H_
#define SYSTEMMESSAGE_H_

#include <string>
#include "Message.h"

class SystemMessage: public Message {
private:
	std::string m_szVersion;
	std::string m_szPlatform;

protected:
	void Init();

public:
	SystemMessage();
	virtual ~SystemMessage();
};

#endif /* SYSTEMMESSAGE_H_ */

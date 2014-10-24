/*
 * IMessage.h
 *
 *  Created on: 2014-6-28
 *      Author: manson
 */

#ifndef IMESSAGE_H_
#define IMESSAGE_H_

class IMessage {
public:
	IMessage() {
	}
	virtual ~IMessage() {
	}
	virtual std::string GetMessage() = 0;
	virtual void Log() = 0;
};

#endif /* IMESSAGE_H_ */

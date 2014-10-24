/*
 * Message.cpp
 *
 *  Created on: 2014-6-28
 *      Author: manson
 */

#include "message/Message.h"
#include "json/json.h"
#include "utils/utils.h"
#include <sstream>

using namespace std;

Message::Message() {
}

Message::~Message() {
}

string Message::GetMessage() {
	return m_szMessage;
}

string Message::GetType() {
	return m_szType;
}

string Message::GetData() {
	return m_szData;
}

void Message::JoinElements() {
	stringstream ss;

	ss << "{";
	ss << "\"type\"";
	ss << ":";
	ss << "\"";
	ss << m_szType;
	ss << "\"";
	ss << ", ";
	ss << "\"data\"";
	ss << ":";
	ss << m_szData;
	ss << "}";
	string msg = ss.str();

	ss.str("");
	ss << msg.size();
	ss << ":";
	ss << msg;

	m_szMessage = ss.str();

	if (CHECK_JSON_FORMAT) {
		Json::Reader reader;
		Json::StyledWriter styled_writer;
		Json::Value val;
		if (!reader.parse(m_szMessage, val)) {
			LOGD("Illegal json format : %s", m_szMessage.c_str());
		} else {
			LOGD("Good json format : %s", m_szMessage.c_str());
		}
	}
}

void Message::Log() {
	LOGD("message = %s", m_szMessage.c_str());
}

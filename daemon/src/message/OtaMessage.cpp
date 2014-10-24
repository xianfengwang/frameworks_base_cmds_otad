/*
 * OtaMessage.cpp
 *
 *  Created on: 2014-6-7
 *      Author: manson
 */

#include "message/OtaMessage.h"
#include "platform/PropertyManager.h"
#include <sstream>

using namespace std;

OtaMessage::OtaMessage(std::string event, std::string result) {
	m_szType = "OTA_UPDATE";
	m_szEvent = event;
	m_szResult = result;

	Init();
	JoinElements();
}

OtaMessage::~OtaMessage() {
}

void OtaMessage::Init() {
	stringstream ss;

	ss << "{";
	ss << "\"event\"";
	ss << ":";
	ss << "\"";
	ss << m_szEvent;
	ss << "\"";
	ss << ", ";
	ss << "\"result\"";
	ss << ":";
	ss << "\"";
	ss << m_szResult;
	ss << "\"";

	if (m_szResult == "SUCCESS") {
		PropertyManager *pm = new PropertyManager();
		string version = pm->GetProperty("ro.product.version",
				"unknow version");
		string platform = pm->GetProperty("ro.product.platform",
				"unknow platform");
		delete pm;

		ss << ", ";
		ss << "\"version\"";
		ss << ":";
		ss << "\"";
		ss << version;
		ss << "\"";
		ss << ", ";
		ss << "\"platform\"";
		ss << ":";
		ss << "\"";
		ss << platform;
		ss << "\"";
	}

	ss << "}";

	m_szData = ss.str();
}

//void OtaMessage::IncreaseSentCounter() {
//	m_nSentCounter++;
//}
//
//void OtaMessage::OnProcess() {
//	m_tTimeStamp = time(NULL);
//}
//
//bool OtaMessage::CheckSentCounter() {
//	return (m_nSentCounter <= RETRY_COUNT);
//}
//
//bool OtaMessage::CheckSentInterval() {
//	if (m_nSentCounter == 0) {
//		return true;
//	}
//
//	time_t temp;
//	temp = time(NULL);
//	double interval = difftime(temp, m_tTimeStamp);
//	return (interval >= RETRY_INTERVAL);
//}

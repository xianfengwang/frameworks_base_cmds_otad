/*
 * SystemMessage.cpp
 *
 *  Created on: 2014-6-28
 *      Author: manson
 */

#include "message/SystemMessage.h"
#include "platform/PropertyManager.h"
#include <sstream>

using namespace std;

SystemMessage::SystemMessage() {
	m_szType = "SYSTEM_STATUS";

	Init();
	JoinElements();
}

SystemMessage::~SystemMessage() {
}

void SystemMessage::Init() {
	PropertyManager *pm = new PropertyManager();
	m_szVersion = pm->GetProperty("ro.product.version", "unknow version");
	m_szPlatform = pm->GetProperty("ro.product.platform", "unknow platform");
	delete pm;

	stringstream ss;
	ss << "{";
	ss << "\"version\"";
	ss << ":";
	ss << "\"";
	ss << m_szVersion;
	ss << "\"";
	ss << ", ";
	ss << "\"platform\"";
	ss << ":";
	ss << "\"";
	ss << m_szPlatform;
	ss << "\"";
	ss << "}";

	m_szData = ss.str();
}


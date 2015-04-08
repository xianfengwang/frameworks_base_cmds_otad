#include "platform/DeviceInfo.h"
#include "platform/PropertyManager.h"
#include "utils/utils.h"
#include "json/json.h"
#include <string>

#include <sys/socket.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <netinet/in.h>
#include <arpa/inet.h>



using namespace std;

DeviceInfo::DeviceInfo() {
}

DeviceInfo::~DeviceInfo() {

}

bool DeviceInfo::LoadFromString(string info) {
	bool res = false;
	Json::Reader reader;
	if (reader.parse(info, m_jsonInfo)) {
		m_szDeviceId = m_jsonInfo[KEY_SERIAL_NUMBER].asString();
		m_szProductName = m_jsonInfo[KEY_PRODUCT_NAME].asString();
		m_szManufacturer = m_jsonInfo[KEY_MANUFACTURER].asString();
		m_szHardwareVersion = m_jsonInfo[KEY_HARDWARE_VERSION].asString();
		m_szVersion = m_jsonInfo[KEY_VERSION].asString();
		m_szMacAddress = m_jsonInfo[KEY_MAC_ADDRESS].asString();
//		m_szIpAddress = m_jsonInfo[KEY_IP_ADDRESS].asString();
		m_szPlatform = m_jsonInfo[KEY_PLATFORM].asString();

		res = true;
	}
	return res;
}

bool DeviceInfo::LoadFromProperty() {

//	m_szDeviceId = u_getValueFromProcFile("/proc/rknand", "SN =",
//			strlen("SN =") + 1, "unknow");
	m_szDeviceId = m_PropertyManager.GetProperty("ro.serialno", "unknow");
	m_szProductName = m_PropertyManager.GetProperty("ro.product.name",
			"unknow");
	m_szManufacturer = m_PropertyManager.GetProperty("ro.product.manufacturer",
			"unknow");
	m_szHardwareVersion = u_getValueFromProcFile("/proc/rknand", "HWINF =",
			strlen("HWINF =") + 1, "unknow");
	m_szVersion = m_PropertyManager.GetProperty("ro.product.version", "unknow");
	m_szMacAddress = u_getValueFromProcFile("/sys/class/net/wlan0/address",
			"unknow");

	//this maybe failed when otad first launched since network is bad
//	m_szIpAddress = u_getDeviceIp();

	m_szPlatform = m_PropertyManager.GetProperty("ro.product.platform",
			"unknow");

	m_jsonInfo[KEY_SERIAL_NUMBER] = m_szDeviceId;
	m_jsonInfo[KEY_PRODUCT_NAME] = m_szProductName;
	m_jsonInfo[KEY_MANUFACTURER] = m_szManufacturer;
	m_jsonInfo[KEY_HARDWARE_VERSION] = m_szHardwareVersion;
	m_jsonInfo[KEY_VERSION] = m_szVersion;
	m_jsonInfo[KEY_MAC_ADDRESS] = m_szMacAddress;
//	m_jsonInfo[KEY_IP_ADDRESS] = m_szIpAddress;
	m_jsonInfo[KEY_PLATFORM] = m_szPlatform;

	return true;
}

Json::Value DeviceInfo::GetOTAJsonValue() {

	m_otaJsonInfo[KEY_SERIAL_NUMBER] = m_szDeviceId;
	m_otaJsonInfo[KEY_PRODUCT_NAME] = m_szProductName;
	m_otaJsonInfo[KEY_MANUFACTURER] = m_szManufacturer;
	m_otaJsonInfo[KEY_HARDWARE_VERSION] = m_szHardwareVersion;
	m_otaJsonInfo[KEY_VERSION] = m_szVersion;
	m_otaJsonInfo[KEY_MAC_ADDRESS] = m_szMacAddress;
	m_otaJsonInfo[KEY_IP_ADDRESS] = getIp();
	m_otaJsonInfo[KEY_PLATFORM] = m_szPlatform;
	m_otaJsonInfo[KEY_TIME] = getTime();
	m_otaJsonInfo[KEY_ALIVE_TIME] = getAliveTime();
	return m_otaJsonInfo;
}

std::string DeviceInfo::getIp() {
	int sockfd;
	string str;
	char lanip[16];
	struct ifreq ifr;
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd <= 0) {
		std::cerr << "Can not create socket" << std::endl;
		return "";
	}
	memset(&ifr, 0, sizeof(struct ifreq));

	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, "wlan0", IFNAMSIZ - 1);
	ioctl(sockfd, SIOCGIFADDR, &ifr);
	close(sockfd);

	sprintf(lanip, "%s",
			inet_ntoa(((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr));
	str = inet_ntoa(((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr);
	return str;
}

void DeviceInfo::Log() {
	LOGD("device info: \n %s", Dump2JsonStr().c_str());
}

string DeviceInfo::Dump2JsonStr() {
	return m_jsonInfo.toStyledString();
}

Json::Value DeviceInfo::GetJsonValue() {
	return m_jsonInfo;
}

string DeviceInfo::GetDeviceId() {
	return m_szDeviceId;
}

string DeviceInfo::GetProductName() {
	return m_szProductName;
}

string DeviceInfo::GetManufacturer() {
	return m_szManufacturer;
}

string DeviceInfo::GetHardwareVersion() {
	return m_szHardwareVersion;
}

string DeviceInfo::GetVersion() {
	return m_szVersion;
}

string DeviceInfo::GetMacAddress() {
	return m_szMacAddress;
}

//string DeviceInfo::GetIpAddress() {
//	return m_szIpAddress;
//}

string DeviceInfo::GetPlatform() {
	return m_szPlatform;
}

std::string DeviceInfo::GetTime() {
	return m_szTime;
}
std::string DeviceInfo::GetAliveTime() {
	return m_szAliveTime;
}

int DeviceInfo::getTime() {
	time_t tm;
	time(&tm);
	return tm;
}

int DeviceInfo::getAliveTime() {
	struct sysinfo s_info;
	long uptime = 0;
	if (0 == sysinfo(&s_info)) {
		uptime = s_info.uptime;
	}
	return uptime;
}


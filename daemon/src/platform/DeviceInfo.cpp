#include "platform/DeviceInfo.h"
#include "platform/PropertyManager.h"
#include "utils/utils.h"
#include "json/json.h"
#include <string>

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
	m_szDeviceId = u_getValueFromProcFile("/proc/rknand", "SN =",
			strlen("SN =") + 1, "unknow");
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

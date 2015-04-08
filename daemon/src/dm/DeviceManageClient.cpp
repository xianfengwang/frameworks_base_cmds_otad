#include "dm/DeviceManageClient.h"
#include "json/json.h"
#include <iostream>
#include <fstream>
#include "utils/utils.h"
#include "curl/curl.h"

DeviceManageClient::DeviceManageClient() {
	m_DeviceInfo = new DeviceInfo();
	m_DeviceInfo->LoadFromProperty();
}

DeviceManageClient::~DeviceManageClient() {
	delete m_DeviceInfo;
}

static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	LOGD("DeviceManageClient post result: %s",(char*)buffer);
    return nmemb;
}

void DeviceManageClient::write_json() {
	LOGD("DeviceManageClient write_json");
	if (m_DeviceInfo) {
		Json::FastWriter fast_writer;
		LOGD("DeviceManageClient JSON: %s",fast_writer.write(m_DeviceInfo->GetOTAJsonValue()).c_str());
		ofstream out;
		out.open("/data/test.json", ios::out | ios::app);
		if (out.is_open()) {
			out << fast_writer.write(m_DeviceInfo->GetOTAJsonValue()).c_str();
		}
		out.flush();
		out.close();
	} else {
		LOGE("DeviceManageClient m_DeviceInfo NULL");
	}
}

void DeviceManageClient::clear_json() {
	LOGD("DeviceManageClient: clear JSON file");
	fstream file;
	file.open("/data/test.json", ios::out | ios_base::trunc);
	file.close();
}

void DeviceManageClient::post_file(const std::string url,
		const std::string file_path) {

	CURL *curl;
	CURLcode res;
	double speed_upload, total_time;
	int responseCode = -1;

	m_DeviceInfo->LoadFromProperty();

	string device_id = m_DeviceInfo->GetDeviceId();
	string device_mac = m_DeviceInfo->GetMacAddress();

	string stropt = "mac:" + device_mac + "&" + "sn:" + device_id;
	LOGE("DeviceManageClient: strOpt = %s", stropt.c_str());

	curl = curl_easy_init();
	if (curl) {
		struct curl_httppost *formpost = NULL;
		struct curl_httppost *lastptr = NULL;
		curl_formadd(&formpost, &lastptr,
				CURLFORM_PTRNAME, "sn",
				CURLFORM_PTRCONTENTS, device_id.c_str(),
				CURLFORM_END);

		curl_formadd(&formpost, &lastptr,
				CURLFORM_PTRNAME, "mac",
				CURLFORM_PTRCONTENTS, device_mac.c_str(),
				CURLFORM_END);

		curl_formadd(&formpost, &lastptr,
				CURLFORM_PTRNAME, "api_key",
				CURLFORM_PTRCONTENTS, "api$user",
				CURLFORM_END);

		curl_formadd(&formpost, &lastptr,
				CURLFORM_PTRNAME, "api_secret",
				CURLFORM_PTRCONTENTS, "api$secret*china(mobile@2011",
				CURLFORM_END);

		curl_formadd(&formpost, &lastptr,
				CURLFORM_PTRNAME, "log",
				CURLFORM_FILE, file_path.c_str(),
				CURLFORM_END);

		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
		res = curl_easy_perform(curl);

		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
		if (res != CURLE_OK || responseCode != 200) {
			LOGE("DeviceManageClient post failed: %s", curl_easy_strerror(res));
		} else {
			clear_json();
			LOGE("DeviceManageClient responseCode = %d", responseCode);
			LOGE("DeviceManageClient res = %d", res);
			curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD, &speed_upload);
			curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total_time);
			LOGE("DeviceManageClient: Speed: %.3f bytes/sec during %.3f seconds",
					speed_upload, total_time);
		}
		curl_easy_cleanup(curl);
		curl_formfree(formpost);
	}
	curl = NULL;
}

#include "download/DownloadTask.h"
#include "utils/utils.h"
#include "fs/fs_utils.h"
#include "json/json.h"
#include <string>
#include <stdlib.h>

using namespace std;

DownloadTask::DownloadTask() {
	m_dFileSize = 0;
	m_dProgress = 0;
	m_dBreakpoint = 0;
}

DownloadTask::DownloadTask(string url, string localPath, string backupPath,
		string md5) {
	m_szUrl = url;
	m_szLocalPath = localPath;
	m_szBackupPath = backupPath;
	m_szMd5 = md5;
	m_dFileSize = 0;
	m_dProgress = 0;
	m_dBreakpoint = 0;
}

DownloadTask::~DownloadTask() {
}

void DownloadTask::Backup() {
	fs_WriteFile(m_szBackupPath, Dump2JsonStr());
}

string DownloadTask::Dump2JsonStr() {
	Json::Value root;

	root["url"] = GetUrl();
	root["local_path"] = GetLocalPath();
	root["backup_path"] = GetBackupPath();
	root["md5"] = GetMd5();
	root["total_size"] = Json::Value(GetFileSize());
	root["progress_size"] = Json::Value(GetProgress());

	return root.toStyledString();
}

bool DownloadTask::LoadFromJsonFile(string path) {
	string strJson = fs_ReadFile(path);
	Json::Reader reader;
	Json::StyledWriter styled_writer;
	Json::Value val;
	if (!reader.parse(strJson, val)) {
		LOGD("download task parse failed : %s", strJson.c_str());
		return false;
	} else {
//		LOGD("parse OK = %s", val.toStyledString().c_str());
		m_szUrl = val["url"].asString();
		m_szLocalPath = val["local_path"].asString();
		m_szBackupPath = val["backup_path"].asString();
		m_szMd5 = val["md5"].asString();
		m_dFileSize = val["total_size"].asDouble();
		m_dProgress = val["progress_size"].asDouble();
	}
	return true;
}

void DownloadTask::Log() {
	LOGD("local path = %s", m_szLocalPath.c_str());
	LOGD("url = %s", m_szUrl.c_str());
	LOGD("backupPath = %s", m_szBackupPath.c_str());
	LOGD("md5 = %s", m_szMd5.c_str());
	LOGD("file size = %f", m_dFileSize);
	LOGD("progress = %f", m_dProgress);
}

string DownloadTask::GetBackupPath() {
	return m_szBackupPath;
}

string DownloadTask::GetUrl() {
	return m_szUrl;
}

string DownloadTask::GetMd5() {
	return m_szMd5;
}

string DownloadTask::GetLocalPath() {
	return m_szLocalPath;
}

double DownloadTask::GetFileSize() {
	return m_dFileSize;
}

void DownloadTask::SetFileSize(double size) {
	m_dFileSize = size;
}

double DownloadTask::GetProgress() {
	return m_dProgress;
}

void DownloadTask::SetProgress(double progress) {
	m_dProgress = progress;
}

double DownloadTask::GetBreakpoint() {
	return m_dBreakpoint;
}

void DownloadTask::SetBreakpoint(double progress) {
	m_dBreakpoint = progress;
}

#ifndef _DOWNLOAD_TASK_H_
#define _DOWNLOAD_TASK_H_

#include <string>

class DownloadTask {
public:
	DownloadTask();
	DownloadTask(std::string url, std::string localPath, std::string backupPath,
			std::string md5);
	virtual ~DownloadTask();

	std::string GetLocalPath();
	std::string GetUrl();
	std::string GetBackupPath();
	std::string GetMd5();
	double GetFileSize();
	double GetProgress();
	void SetFileSize(double size);
	void SetProgress(double progress);
	void SetBreakpoint(double progress);
	double GetBreakpoint();
	void Backup();
	std::string Dump2JsonStr(); //dump the object to an json object
	bool LoadFromJsonFile(std::string path);
	void Log();
private:
	std::string m_szLocalPath;
	std::string m_szUrl;
	std::string m_szBackupPath;
	std::string m_szMd5;
	double m_dFileSize;
	double m_dProgress;
	double m_dBreakpoint;
};

#endif

#ifndef _DOWNLOADER_H_
#define _DOWNLOADER_H_

#include "DownloadTask.h"
#include <string>
#include <pthread.h>

#define DOWNLOAD_RETRY_TIMES 		10
#define DOWNLOAD_RETRY_INTERVAL 	5
#define CONNECTION_TIMEOUT			60

class Downloader {
public:
	class Callback {
	public:
		Callback() {
		}
		virtual ~Callback() {
		}

		virtual void onDownloadStart(Downloader *downloader) = 0;
		virtual void onDownloadInit(Downloader *downloader,
				double totalSize) = 0;
		virtual void onDownloadRetry(Downloader *downloader, int counter) = 0;
		virtual void onDownloadFailed(Downloader *downloader,
				std::string cause) = 0;
		virtual void onDownloadSuccess(Downloader *downloader) = 0;
		virtual void onDownloadProgress(Downloader *downloader, double total,
				double progress) = 0;
	};

	Downloader();
	virtual ~Downloader();

	DownloadTask * GetDownloadTask();
	void SetDownloadTask(DownloadTask *task);
	Callback * GetCallback();
	void SetCallback(Callback *callback);
	pthread_t GetWorkerId();

	bool Run();
	bool Interrupt();

	double RequireFileSize();
	bool DownloadFile();

	bool IsSuccess();
	void Clean(); //clean member variable
private:
	DownloadTask *m_TaskInfo;
	Callback *m_Callback;
	pthread_t m_ThreadId;
	bool m_bResult;
};

#endif

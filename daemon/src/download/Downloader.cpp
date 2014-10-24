#include "download/Downloader.h"
#include "utils/utils.h"
#include "fs/fs_utils.h"
#include "curl/curl.h"

#include <errno.h>

using namespace std;

size_t saveToFile(char *buffer, size_t size, size_t nmemb, void *userdata);
int onDownloadProgress(void* p, double dtotal, double dnow, double ultotal,
		double ulnow);

Downloader::Downloader() {
	m_TaskInfo = NULL;
	m_Callback = NULL;
	m_ThreadId = 0;
	m_bResult = false;
}

Downloader::~Downloader() {
}

DownloadTask* Downloader::GetDownloadTask() {
	return m_TaskInfo;
}

void Downloader::SetDownloadTask(DownloadTask* task) {
	m_TaskInfo = task;
}

Downloader::Callback *Downloader::GetCallback() {
	return m_Callback;
}

void Downloader::SetCallback(Callback *callback) {
	m_Callback = callback;
}

pthread_t Downloader::GetWorkerId() {
	return m_ThreadId;
}

void thread_exit_handler(int sig) {
	LOGD("thread_exit_handler get signal is %d \n", sig);
	pthread_exit(0);
}

//work thread
void * thread_worker(void *p) {
	struct sigaction actions;
	memset(&actions, 0, sizeof(actions));
	sigemptyset(&actions.sa_mask);
	actions.sa_flags = 0;
	actions.sa_handler = thread_exit_handler;
	int rc = sigaction(SIGUSR1, &actions, NULL);
	if (rc != 0) {
		LOGE("set SIGUSR1 action failed, exit thread");
		return NULL;
	}

	if (p == NULL) {
		LOGE("pointer param is NULL");
		return NULL;
	}

	int retryCounter = 0;
	Downloader *downloader = (Downloader *) p;
	downloader->GetCallback()->onDownloadStart(downloader);
	do {
		double size = downloader->RequireFileSize();
		if (size < 0) {
			sleep(retryCounter * DOWNLOAD_RETRY_INTERVAL);
			retryCounter++;
			if (retryCounter <= DOWNLOAD_RETRY_TIMES) {
				downloader->GetCallback()->onDownloadRetry(downloader,
						retryCounter);
				continue;
			} else {
				//callback failed
				downloader->GetCallback()->onDownloadFailed(downloader,
						"require file size failed!");
				return NULL;
			}
		} else {
			LOGD("require file size OK: continue to download");
			downloader->GetCallback()->onDownloadInit(downloader, size);
			break;
		}
	} while (true);

	retryCounter = 0;
	do {
		bool success = downloader->DownloadFile();
		if (!success) {
			sleep(retryCounter * DOWNLOAD_RETRY_INTERVAL);
			retryCounter++;
			if (retryCounter <= DOWNLOAD_RETRY_TIMES) {
				//callback retry
				downloader->GetCallback()->onDownloadRetry(downloader,
						retryCounter);
				continue;
			} else {
				//callback failed
				downloader->GetCallback()->onDownloadFailed(downloader,
						"download failed!");
				return NULL;
			}
		} else {
			//ensure file is complete
			sync();
			sleep(3);
			//callback success
			downloader->GetCallback()->onDownloadSuccess(downloader);
			break;
		}
	} while (true);

	return NULL;
}

bool Downloader::Run() {
	if (m_TaskInfo == NULL) {
		LOGD("task info cannot be null");
		return false;
	}
	if (m_Callback == NULL) {
		LOGD("callback cannot be null");
		return false;
	}
	int returnVal = pthread_create(&m_ThreadId, NULL, thread_worker, this);
	LOGD("Create worker id = %ld", m_ThreadId);
	if (returnVal) {
		LOGD("Create worker failed : reason = %d", returnVal);
		return false;
	}
	return true;
}

bool Downloader::Interrupt() {
	bool res = false;
	int result = 0;
	if ((result = pthread_kill(m_ThreadId, SIGUSR1)) != 0) {
		if (ESRCH == result) {
			LOGD("interrupt worker %ld failed : work is already finished!",
					m_ThreadId);
			res = true;
		} else {
			LOGD("interrupt worker %ld failed : reason = %d", m_ThreadId,
					result);
			res = false;
		}
	} else {
		res = true;
	}

	Clean();
	return res;
}

double Downloader::RequireFileSize() {
	CURL *curl;
	char errbuf[CURL_ERROR_SIZE] = { 0 };
	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, m_TaskInfo->GetUrl().c_str());
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
		curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
		curl_easy_setopt(curl, CURLOPT_HEADER, 1);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CONNECTION_TIMEOUT);
		CURLcode ret = curl_easy_perform(curl);

		long responseCode = 0;
		double size = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
		curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &size);
		curl_easy_cleanup(curl);
		if (CURLE_OK == ret && responseCode == 200) {
			return size;
		} else {
			LOGD("err info = %s", errbuf);
			LOGD("response code = %ld", responseCode);
			m_bResult = false;
			return -1;
		}
	}
	return -1;
}

bool Downloader::DownloadFile() {
	CURL *curl;
	FILE *fp = NULL;
	char errbuf[CURL_ERROR_SIZE] = { 0 };
	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, m_TaskInfo->GetUrl().c_str());
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);

		double breakpoint = m_TaskInfo->GetProgress();
		if (breakpoint == m_TaskInfo->GetFileSize()) { //already download finished
			return true;
		}
		m_TaskInfo->SetBreakpoint(breakpoint);
		if (breakpoint > 0) {
			fp = fopen(m_TaskInfo->GetLocalPath().c_str(), "a");
			curl_easy_setopt(curl, CURLOPT_RESUME_FROM, (long )breakpoint);
		} else {
			fp = fopen(m_TaskInfo->GetLocalPath().c_str(), "w");
		}

		if (fp == NULL) {
			LOGD("fp is NULL");
			m_bResult = false;
			return false;
		}

		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void * ) fp);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, saveToFile);

		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, onDownloadProgress);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CONNECTION_TIMEOUT);
		CURLcode ret = curl_easy_perform(curl);

		long responseCode = 0;
		double size;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
		curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &size);
		curl_easy_cleanup(curl);
		fclose(fp);

		if (CURLE_OK == ret && (responseCode == 200 || responseCode == 206)
				&& (m_TaskInfo->GetFileSize() == (size + breakpoint))) {
			LOGD("download OK");
			m_bResult = true;
			return true;
		} else {
			LOGD("err info = %s", errbuf);
			LOGD("response code = %ld", responseCode);
			m_bResult = false;
			return false;
		}
	}
	return false;
}

bool Downloader::IsSuccess() {
	return m_bResult;
}

/*
 * if reuse the downloader, you must call
 * 1.clean
 * 2.setXXX
 * 3.run
 */
void Downloader::Clean() {
	m_TaskInfo = NULL;
	m_Callback = NULL;
	m_ThreadId = 0;
}

int onDownloadProgress(void* p, double total, double now, double ultotal,
		double ulnow) {
	if (p == NULL) {
		return -1;
	}
	Downloader *downloader = (Downloader *) p;
	if (downloader != NULL) {
//		downloader->GetDownloadTask()->SetFileSize(total);
		double breakPoint = downloader->GetDownloadTask()->GetBreakpoint();
		downloader->GetDownloadTask()->SetProgress(breakPoint + now);
		downloader->GetDownloadTask()->Backup();
		downloader->GetCallback()->onDownloadProgress(downloader, total, now);
	}
	return 0;
}

size_t saveToFile(char *buffer, size_t size, size_t nmemb, void *userdata) {
	FILE *fp = (FILE *) userdata;
	size_t return_size = fwrite(buffer, size, nmemb, fp);
	fflush(fp);
	return return_size;
}

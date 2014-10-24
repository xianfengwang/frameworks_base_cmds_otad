/*
 * DownloadCallback.h
 *
 *  Created on: 2014-6-7
 *      Author: manson
 */

#ifndef DOWNLOADCALLBACK_H_
#define DOWNLOADCALLBACK_H_

#include "Downloader.h"
#include "fs/fs_utils.h"
#include <string>
#include "platform/ProcessMonitor.h"

class DownloadCallback: public Downloader::Callback {
private:
	int counter;
public:
	DownloadCallback() {
		counter = 0;
	}
	~DownloadCallback() {
	}

	void onDownloadRetry(Downloader *downloader, int count) {
		LOGD("CALLBACK: download retry, counter = %d", count);
	}

	void onDownloadStart(Downloader *downloader) {
		LOGD("CALLBACK: download start");
	}

	void onDownloadInit(Downloader *downloader, double totalSize) {
		LOGD("CALLBACK: download init");
		DownloadTask *task = downloader->GetDownloadTask();

		bool reset = false;
		//size not match
		if (task->GetFileSize() != totalSize) {
			LOGD("total size not match: server=%f, local=%f", totalSize,
					task->GetFileSize());
			reset = true;
		}

		if (reset) {
			task->SetFileSize(totalSize);
			task->SetProgress(0);
			//delete files
			LOGD("reset download task, delete /mnt/sdcard/fota/*");
			fs_CleanDir(std::string("/mnt/sdcard/fota/"));
		}
	}

	void onDownloadFailed(Downloader *downloader, std::string cause) {
		LOGD("CALLBACK: download failed, cause : %s",
				cause.empty() ? "unknow" : cause.c_str());
		downloader->Clean();
	}

	void onDownloadSuccess(Downloader *downloader) {
		LOGD("CALLBACK: download success");
	}

	void onDownloadProgress(Downloader *downloader, double total,
			double progress) {
		counter++;
		if ((counter % 500) == 0 || (progress == total)) {
			LOGD(
					"CALLBACK: download progress: total=%f, progress=%f, percent=%%%f",
					total, progress, (progress * 100) / total);
			if (ProcessMonitor::GetInstance()->CheckCertainProcessAlive(
					"CastAppContaine")) {
//                LOGD("-------------sleep 1---------------");
				sleep(1);
			}
		}
	}
};

#endif /* DOWNLOADCALLBACK_H_ */

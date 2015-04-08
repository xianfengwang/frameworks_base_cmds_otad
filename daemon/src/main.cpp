#include "utils/utils.h"
#include "platform/DeviceInfo.h"
#include "download/DownloadTask.h"
#include "download/Downloader.h"
#include "download/DownloadCallback.h"
#include "fs/fs_utils.h"
#include "json/json.h"
#include "MutexQueue.h"
#include "platform/RecoveryManager.h"
#include "net/HttpClient.h"
#include "curl/curl.h"
#include "net/RequestParams.h"
#include "net/RequireResult.h"
#include "message/Message.h"
#include "message/OtaMessage.h"
#include "message/SystemMessage.h"
#include "message/IMessage.h"
#include "dm/DeviceManageClient.h"

#include <pthread.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define SERVER_SOCKET_PORT 		4999
#define UPDATE_CHECK_INTERVAL	60 //should be longer in product mode
#define RECOVERY_PATH_PREFIX	"/cache/recovery/"
#define FOTA_PATH_PREFIX		"/mnt/internal_sd/fota/"
#define DOWNLOAD_TASKINFO_PATH	"/mnt/internal_sd/fota/task_info.txt"

#define OTA_SERVER_API 			"http://ota.infthink.com/cast/check"

#define EVENT_CLEAR_MSG			"EVENT_CLEAR_MSG"
#define EVENT_UPGRADE			"EVENT_UPGRADE"
#define EVENT_DOWNLOAD			"EVENT_DOWNLOAD"
#define RESULT_SUCCESS			"SUCCESS"
#define RESULT_FAIL				"FAIL"

#define DEBUG					true
#define TEST					false
#define ABORT					true
#define LAUNCH_CHECKER			true
#define LAUNCH_SOCKET			true
#define LAUNCH_DM_CLIENT		true

using namespace std;

void * dm_upload_thread(void *argv);
void * check_thread(void *argv);
RequireResult * requireUpdatePackage();
DownloadTask * createDownloadTask(RequireResult *rr);
string findLocalAvailableUpdatePackage();

void notifyHome(IMessage *msg);
void notifyClear();
void notifyUpgradeResult(bool result);
void notifyDownloadResult(bool result);
void notifySystemStatus();

void clearBootMessage();
void writeBootMessage(string path);

void setSockOption(int sockServer);
bool handshakeWithHome(int fd);
void * sock_server_thread(void *argv);
void fake_sock_client();

void test();
int dm_check_tiem = 0;

MutexQueue<IMessage> *g_MessageQueue;

int main() {
	LOGD("--------------OTA running--------------1.0");
	//0.create sender queue and notify system status
	g_MessageQueue = new MutexQueue<IMessage>(1024);

	if (TEST) {
		test();
		if (ABORT) {
			return 0;
		}
	}

	//1.check local dir
	if (!fs_IsDirExist(FOTA_PATH_PREFIX)) {
		bool res = fs_MkDir(FOTA_PATH_PREFIX);
		if (!res) {
			LOGD("create %s dir failed, Abort OTAD.", FOTA_PATH_PREFIX);
			return -1;
		}
	}

	//2.check last update result
	if (RecoveryManager::IsRecoverySuccess()) {
		LOGD("!!!!!!UPGRADE SUCCESS!!!!!!");
		notifyUpgradeResult(true);
		fs_CleanDir(FOTA_PATH_PREFIX);
	} else {
		if (RecoveryManager::IsRecoveryFailed()) {
			LOGD("!!!!!!UPGRADE FAILED!!!!!!");
			notifyUpgradeResult(false);
			fs_CleanDir(FOTA_PATH_PREFIX);
		}
		notifySystemStatus();
	}

	int err;
	//3.start socket server
	if (LAUNCH_SOCKET) {
		pthread_t sockThreadId;
		err = pthread_create(&sockThreadId, NULL, sock_server_thread, NULL);
		if (err != 0) {
			LOGE("launch \"sock_server_thread\" thread error, Abort OTAD.");
			return -1;
		}
	}

	//4.start checker thread
	if (LAUNCH_CHECKER) {
		pthread_t checkerId;
		err = pthread_create(&checkerId, NULL, check_thread, NULL);
		if (err != 0) {
			LOGE("launch \"check_update\" thread error, Abort OTAD.");
			return -1;
		}
	}

	//5.dm_client
	if (LAUNCH_DM_CLIENT) {
		pthread_t dmId;
		err = pthread_create(&dmId, NULL, dm_upload_thread, NULL);
		if (err != 0) {
			LOGE("launch \"dm_upload_thread\" thread error, Abort OTAD.");
			return -1;
		}
	}

	//6.looper
	while (true) {
		sleep(10);
	}

	return 0;
}

void test() {
	notifyUpgradeResult(true);
	notifyUpgradeResult(false);
	notifyDownloadResult(true);
	notifyDownloadResult(false);
	notifyClear();
	notifySystemStatus();
	LOGD("queue size = %d", g_MessageQueue->Size());
}

void setSockOption(int sockServer) {
	int iReuseAddr = SO_REUSEADDR;
	setsockopt(sockServer, SOL_SOCKET, SO_REUSEADDR, (void *) &iReuseAddr,
			sizeof(iReuseAddr));
	int keepAlive = 1;
	setsockopt(sockServer, SOL_SOCKET, SO_KEEPALIVE, (void *) &keepAlive,
			sizeof(keepAlive));
//	int keepIdle = 10 * 60;
//	setsockopt(sockServer, SOL_TCP, TCP_KEEPIDLE, (void*) &keepIdle,
//			sizeof(keepIdle));
//	int keepInterval = 5;
//	setsockopt(sockServer, SOL_TCP, TCP_KEEPINTVL, (void *) &keepInterval,
//			sizeof(keepInterval));
//	int keepCount = 3;
//	setsockopt(sockServer, SOL_TCP, TCP_KEEPCNT, (void *) &keepCount,
//			sizeof(keepCount));
	struct linger mLinger;
	mLinger.l_onoff = 1;
	mLinger.l_linger = 5;
	setsockopt(sockServer, SOL_SOCKET, SO_LINGER, &mLinger, sizeof(mLinger));
}

bool handshakeWithHome(int fd) {
	char buf[1024] = { 0 };
	string received;
	int recvLen = recv(fd, buf, 1024, 0);
	if (recvLen > 0) {
		buf[recvLen] = '\0';
		received.append(buf);
	} else {
		return false;
	}

	LOGD("HANDSHAKE : %s", received.c_str());

	Json::Reader reader;
	Json::StyledWriter styled_writer;
	Json::Value val;
	if (!reader.parse(received, val)) {
		return false;
	}

	if (val["type"] != "HANDSHAKE" || val["data"] != "ota_client") {
		return false;
	}
	return true;
}

void * dm_upload_thread(void *argv) {
	LOGD("dm_upload_thread");
	while (true) {

		struct stat file_info;
		FILE *fd;
		curl_off_t fsize;

		fd = fopen("/data/test.json", "a+");
		if (fd != NULL && fstat(fileno(fd), &file_info) == 0) {
			fclose(fd);
			fsize = (curl_off_t) file_info.st_size;
			LOGD("dm_client log file size: %" CURL_FORMAT_CURL_OFF_T " bytes.\n", fsize);
			LOGD("dm_client check time: %d", dm_check_tiem);
			if (fsize >= 20480 || dm_check_tiem >= 10) {
				DeviceManageClient *dmc = new DeviceManageClient();
				dmc->write_json();
				dmc->post_file("http://10.0.0.201:9002/log/upload",
						"/data/test.json");
				dm_check_tiem = 0;
				delete dmc;
			} else {
				LOGD("file size < 20kb or time < 3min");
			}
		} else {
			LOGE("open JSON file failed");
		}

		LOGD("sleep 5 SEC");
		sleep(5);
		dm_check_tiem += 5;
	}

	return NULL;
}

void * sock_server_thread(void *argv) {
	int socketFd;
	struct sockaddr_in s_add;
	socketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFd == -1) {
		LOGD("create server socket failed");
		return NULL;
	} else {
		LOGD("sockServer:%d ---> create success", socketFd);
	}
	setSockOption(socketFd);

	bzero(&s_add, sizeof(struct sockaddr_in));
	s_add.sin_family = AF_INET;
	s_add.sin_addr.s_addr = inet_addr("127.0.0.1");
	s_add.sin_port = htons(SERVER_SOCKET_PORT);

	if (bind(socketFd, (struct sockaddr *) (&s_add), sizeof(struct sockaddr))
			== -1) {
		LOGD("sockServer:%d ---> bind failed", socketFd);
		close(socketFd);
		return NULL;
	} else {
		LOGD("sockServer:%d ---> bind success", socketFd);
	}

	if (listen(socketFd, 5)) {
		LOGD("sockServer:%d ---> listen failed", socketFd);
		close(socketFd);
		return NULL;
	} else {
		LOGD("sockServer:%d ---> listen success", socketFd);
	}

	int homeFd = 0;
	int maxFd = 0;
	bool first = true;
	while (true) {
		maxFd = (socketFd > homeFd) ? socketFd : homeFd;
		fd_set read_set;
		FD_ZERO(&read_set);
		FD_SET(socketFd, &read_set);

		//select time out, 10 milliseconds
		struct timeval tv;
		tv.tv_sec = 10 / 1000;
		tv.tv_usec = (10 % 1000) * 1000;

		if (select(maxFd + 1, &read_set, NULL, NULL, &tv) > 0) {
			if (FD_ISSET(socketFd, &read_set)) { //check socket server
				struct sockaddr_in client_addr;
				socklen_t length = sizeof(client_addr);
				int clientFd = accept(socketFd, (struct sockaddr*) &client_addr,
						&length);
				if (clientFd < 0) {
					LOGD("sockServer:%d ---> accept failed", socketFd);
				} else {
					LOGD("sockServer:%d ---> accept success", socketFd);
					if (homeFd != 0) {
						close(homeFd);
					}
					homeFd = clientFd;

					//{
					//	"type":"HANDSHAKE",
					//	"data":"ota_client"
					//}
					if (handshakeWithHome(homeFd)) {
						LOGD("HANDESHAKE: OK");
						if (first) {
							first = false;
						} else {
							notifySystemStatus();
						}
					} else {
						LOGD("HANDESHAKE: FAILED");
						close(homeFd);
						homeFd = 0;
						continue;
					}
				}
			}
		}

		if (homeFd != 0) {
			while (g_MessageQueue->Size() > 0) {
				sleep(1);
				LOGD("obtain message begin--------");
				IMessage *message = g_MessageQueue->Front();
				LOGD("obtain message end++++++++++");
				if (message != NULL) {
					string msg = message->GetMessage();
					LOGD("sockServer:%d ---> send:\n%s", socketFd, msg.c_str());
					if (send(homeFd, msg.c_str(), msg.size(), 0) < 0) {
						LOGD("sockServer:%d ---> send failed", socketFd);
						break;
					} else {
						g_MessageQueue->Pop();
						delete message;
						LOGD("sockServer:%d ---> send success", socketFd);
					}
				}
			}
		}
	}

	close(socketFd);
	return NULL;
}

void * check_thread(void *argv) {
	sleep(30); //wait for network connection

	LOGD("--------------checker thread running--------------");
	long counter = 0;
	while (true) {
		if (DEBUG) {
			counter++;
			LOGD("checker run <%ld> times", counter);
		}
		//0. http request check
		RequireResult *rr = requireUpdatePackage();
		if (rr == NULL) {
			sleep(UPDATE_CHECK_INTERVAL);
			continue;
		}

		//1. create download task by RequestResult
		DownloadTask *dt = createDownloadTask(rr);
		if (dt == NULL) {
			sleep(UPDATE_CHECK_INTERVAL);
			continue;
		}

		//2. continue downloading or start a new download task
		Downloader *downloader = new Downloader();
		DownloadCallback *cb = new DownloadCallback();
		downloader->SetDownloadTask(dt);
		downloader->SetCallback(cb);
		if (downloader->Run()) {
			notifyClear();
			pthread_t workerId = downloader->GetWorkerId();
			if (workerId != 0) {
				pthread_join(workerId, NULL); //waiting for worker thread finish
			}

			//3. notify home, ota package ready
			if (downloader->IsSuccess()) {
				if (compareMd5(dt->GetLocalPath(), dt->GetMd5())) {
					//4. notify home
					LOGD("DOWNLOAD package success, notify home");
					writeBootMessage(dt->GetLocalPath());
					notifyDownloadResult(true);
				} else {
					clearBootMessage();
					notifyDownloadResult(false);
					if (DEBUG) {
						LOGD("compare md5 failed after download, delete %s",
								FOTA_PATH_PREFIX);
					}
					fs_CleanDir(FOTA_PATH_PREFIX);
				}
			} else {
				//5. download failed delete all files
				clearBootMessage();
				notifyDownloadResult(false);
				if (DEBUG) {
					LOGD("download failed, delete %s", FOTA_PATH_PREFIX);
				}
				fs_CleanDir(FOTA_PATH_PREFIX);
			}
		}

		delete rr;
		delete cb;
		delete dt;
		delete downloader;

		sleep(UPDATE_CHECK_INTERVAL);
	}
	return NULL;
}

RequireResult * requireUpdatePackage() {
	RequireResult *rr = NULL;
	RequestParams *rp = new RequestParams();
	if (rp->Load()) {
		HttpClient *hc = new HttpClient();
		string result = hc->PostRequest(OTA_SERVER_API, rp->Dump2String());
		if (result.size() != 0) {
			if (DEBUG) {
				LOGD("receive update check result : %s", result.c_str());
			}
			rr = new RequireResult(result);
			if (!rr->Load()) {
				LOGD("RequestResult load failed! -- %s", result.c_str());
				delete rr;
				rr = NULL;
			} else {
				if (rr->GetStatus() <= 0) {
					delete rr;
					rr = NULL;
				}
			}
		} else {
			LOGD("receive update check result : empty");
			rr = NULL;
		}
		delete hc;
	}
	delete rp;

	if (DEBUG && rr != NULL) {
		rr->Log();
	}
	return rr;
}

DownloadTask * createDownloadTask(RequireResult *rr) {
	if (rr == NULL) {
		return NULL;
	}

	DownloadTask *dt = NULL;
	string localTaskInfoPath = string(DOWNLOAD_TASKINFO_PATH);
	bool isNew = false;
	if (fs_IsFileExist(localTaskInfoPath)) {
		dt = new DownloadTask();
		if (dt->LoadFromJsonFile(localTaskInfoPath)) {
			if (dt->GetUrl() != rr->GetUrl()) { //new url
				if (DEBUG) {
					LOGD("URL not match");
				}
				isNew = true;
			}

			if (dt->GetFileSize() != rr->GetFileSize()) { //total size not match
				if (DEBUG) {
					LOGD("package size not match");
				}
				isNew = true;
			}

			if (!fs_IsFileExist(dt->GetLocalPath())) {
				if (DEBUG) {
					LOGD("local file:%s not exist", dt->GetLocalPath().c_str());
				}
				isNew = true;
			} else {
				double localSize = fs_GetFileSize(dt->GetLocalPath());
				double progress = dt->GetProgress();
				double totalSize = dt->GetFileSize();
				if (DEBUG) {
					LOGD("local file size = %f", localSize);
					LOGD("total file size = %f", totalSize);
					LOGD("task progress size = %f", progress);
				}

				//compare local_size and total_size
				if (localSize != totalSize) {
					if (DEBUG) {
						LOGD("total size not match");
					}
					//compare local_size and task_progress
					if (localSize != dt->GetProgress()) {
						if (DEBUG) {
							LOGD("progress not match");
						}
						isNew = true;
					} else {
						if (DEBUG) {
							LOGD("progress match, should continue download");
						}
						isNew = false;
					}
				} else {
					//compare md5
					if (compareMd5(dt->GetLocalPath(), dt->GetMd5())) {
						if (DEBUG) {
							LOGD("found an available OTA package");
						}
						writeBootMessage(dt->GetLocalPath());
						notifyDownloadResult(true);
						delete dt;
						return NULL;
					} else {
						if (DEBUG) {
							LOGD("package md5 not match");
						}
						isNew = true;
					}
				}
			}
		} else {
			if (DEBUG) {
				LOGD("load task_info.txt failed");
			}
			isNew = true;
		}
	} else {
		if (DEBUG) {
			LOGD("task_info.txt missed");
		}
		isNew = true;
	}

	if (isNew) {
		if (dt != NULL) {
			delete dt;
		}
		if (DEBUG) {
			LOGD("create new download task, delete %s", FOTA_PATH_PREFIX);
		}
		clearBootMessage();
		fs_CleanDir(string(FOTA_PATH_PREFIX));

		string localPath = string(FOTA_PATH_PREFIX);
		localPath += rr->GetFileName();
		dt = new DownloadTask(rr->GetUrl(), localPath, localTaskInfoPath,
				rr->GetMd5());
		dt->SetFileSize(rr->GetFileSize());
		dt->SetProgress(0);
	}

	return dt;
}

string findLocalAvailableUpdatePackage() {
	string pkgPath = string("");
	string localTaskInfoPath = string(DOWNLOAD_TASKINFO_PATH);
	if (fs_IsFileExist(localTaskInfoPath)) {
		DownloadTask *dt = new DownloadTask();
		if (dt->LoadFromJsonFile(localTaskInfoPath)) {
			//1.check file size
			double size = fs_GetFileSize(dt->GetLocalPath());
			if (size == dt->GetFileSize()) {
				//2.check md5
				if (compareMd5(dt->GetLocalPath(), dt->GetMd5())) {
					pkgPath = dt->GetLocalPath();
				} else {
					clearBootMessage();
					fs_CleanDir(string(FOTA_PATH_PREFIX));
				}
			}
		}
		delete dt;
	}
	return pkgPath;
}

void notifyHome(IMessage *msg) {
	LOGD("notify home ---> ");
	g_MessageQueue->Push(msg);
}

void notifyClear() {
	OtaMessage *msg = new OtaMessage(EVENT_CLEAR_MSG, "");
	notifyHome(msg);
}

void notifyUpgradeResult(bool result) {
	OtaMessage *msg = new OtaMessage(EVENT_UPGRADE,
			result ? RESULT_SUCCESS : RESULT_FAIL);
	notifyHome(msg);
}

void notifyDownloadResult(bool result) {
	OtaMessage *msg = new OtaMessage(EVENT_DOWNLOAD,
			result ? RESULT_SUCCESS : RESULT_FAIL);
	notifyHome(msg);
}

void notifySystemStatus() {
	SystemMessage *msg = new SystemMessage();
	notifyHome(msg);
}

void clearBootMessage() {
	RecoveryManager *rm = new RecoveryManager();
	rm->ClearRecoveryCommand();
	delete rm;
}

void writeBootMessage(string path) {
	RecoveryManager *rm = new RecoveryManager();
	rm->WriteRecoveryCommand(path, false);
	delete rm;
}

void fake_sock_client() {
	sleep(10);
	int socketFd;
	struct sockaddr_in s_add;
	socketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFd == -1) {
		LOGD("sockClien : create socket failed");
	} else {
		LOGD("sockClien : sockFD:%d--create socket success", socketFd);
	}

	bzero(&s_add, sizeof(struct sockaddr_in));
	s_add.sin_family = AF_INET;
	s_add.sin_addr.s_addr = inet_addr("127.0.0.1");
	s_add.sin_port = htons(SERVER_SOCKET_PORT);

	if (connect(socketFd, (struct sockaddr *) (&s_add), sizeof(struct sockaddr))
			== -1) {
		LOGD("sockClien : sockFD:%d--connect failed", socketFd);
		close(socketFd);
		return;
	} else {
		LOGD("sockClien : sockFD:%d--connect success", socketFd);
	}

	while (true) {
		char recvBuf[1024] = { 0 };
		int recvLen = recv(socketFd, recvBuf, 1024, 0);
		if (recvLen > 0) {
			recvBuf[recvLen] = '\0';
			LOGD("sockClien : recv = %s", recvBuf);
		} else {
			LOGD("sockClien : sockFD:%d--recv failed", socketFd);
			break;
		}
	}

	close(socketFd);
	return;
}


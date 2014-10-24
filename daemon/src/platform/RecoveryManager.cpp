/*
 * RecoveryManager.cpp
 *
 *  Created on: 2014-5-21
 *      Author: manson
 */

#include "platform/RecoveryManager.h"
#include "librecovery.h"
#include "fs/fs_utils.h"
#include "utils/utils.h"
#include "fs/mtdutils.h"
#include <string>

#include <errno.h>

using namespace std;

string RecoveryManager::s_szRecoverySuccessPath = string("/cache/update.suc");
string RecoveryManager::s_szRecoveryFailedPath = string("/cache/update.err");

int RecoveryManager::UpdateOta(string path, int pathLength) {
	fs_DeleteFile(s_szRecoverySuccessPath);
	return installFotaUpdate(strdup(path.c_str()), pathLength);
}

bool RecoveryManager::IsRecoverySuccess() {
	bool res = false;
	if (fs_IsFileExist(s_szRecoverySuccessPath)) {
		fs_DeleteFile(s_szRecoverySuccessPath);
		res = true;
	}
	return res;
}

bool RecoveryManager::IsRecoveryFailed() {
	bool res = false;
	if (fs_IsFileExist(s_szRecoveryFailedPath)) {
		fs_DeleteFile(s_szRecoveryFailedPath);
		res = true;
	}
	return res;
}

void RecoveryManager::WipeData() {
	struct bootloader_message boot;
	memset(&boot, 0, sizeof(boot));
	strlcpy(boot.command, "boot-recovery", sizeof(boot.command));
	char cmd[100] = "recovery\n--wipe_data";
	strlcpy(boot.recovery, cmd, sizeof(boot.recovery));

	set_bootloader_message_mtd(&boot);
}

void RecoveryManager::WriteRecoveryCommand(string path, bool wipe) {
	struct bootloader_message boot;
	memset(&boot, 0, sizeof(boot));
	strlcpy(boot.command, "boot-recovery", sizeof(boot.command));
	char cmd[256] = "recovery\n--update_package=";
	strcat(cmd, path.c_str());
	if (wipe) {
		strcat(cmd, "\n--wipe_data");
	}
	strlcpy(boot.recovery, cmd, sizeof(boot.recovery));
	LOGD("write misc: path=%s, wipe=%s, command=%s, status=%s, recovery=%s",
			path.c_str(), wipe ? "wipe_data" : "!wipe_data", boot.command,
			boot.status, boot.recovery);
	set_bootloader_message_mtd(&boot);
}

void RecoveryManager::ClearRecoveryCommand() {
	struct bootloader_message boot;
	memset(&boot, 0, sizeof(boot));
	set_bootloader_message_mtd(&boot);
}

int set_bootloader_message_mtd(const struct bootloader_message *in) {
	size_t write_size;
	mtd_scan_partitions();
	const MtdPartition *part = mtd_find_partition_by_name("misc");
	if (part == NULL || mtd_partition_info(part, NULL, NULL, &write_size)) {
		LOGE("Can't find %s\n", "/misc");
		return -1;
	}

	MtdReadContext *read = mtd_read_partition(part);
	if (read == NULL) {
		LOGE("Can't open %s\n(%s)\n", "/misc", strerror(errno));
		return -1;
	}

	ssize_t size = write_size * MISC_PAGES;
	char data[size];
	ssize_t r = mtd_read_data(read, data, size);
	if (r != size)
		LOGE("Can't read %s\n(%s)\n", "/misc", strerror(errno));
	mtd_read_close(read);
	if (r != size)
		return -1;

	memcpy(&data[write_size * MISC_COMMAND_PAGE], in, sizeof(*in));

	MtdWriteContext *write = mtd_write_partition(part);
	if (write == NULL) {
		LOGE("Can't open %s\n(%s)\n", "/misc", strerror(errno));
		return -1;
	}
	if (mtd_write_data(write, data, size) != size) {
		LOGE("Can't write %s\n(%s)\n", "/misc", strerror(errno));
		mtd_write_close(write);
		return -1;
	}
	if (mtd_write_close(write)) {
		LOGE("Can't finish %s\n(%s)\n", "/misc", strerror(errno));
		return -1;
	}

	LOGI("Set boot command \"%s\"\n", in->command[0] != 255 ? in->command : "");
	return 0;
}


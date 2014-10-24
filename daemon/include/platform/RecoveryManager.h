/*
 * RecoveryManager.h
 *
 *  Created on: 2014-5-21
 *      Author: manson
 */

#ifndef RECOVERYMANAGER_H_
#define RECOVERYMANAGER_H_

#include <string>

static const int MISC_PAGES = 3;         // number of pages to save
static const int MISC_COMMAND_PAGE = 1;  // bootloader command is this page

struct bootloader_message {
	char command[32];
	char status[32];
	char recovery[1024];
};

int set_bootloader_message_mtd(const struct bootloader_message *in);

class RecoveryManager {
public:
	int UpdateOta(std::string path, int pathLength);
	void WriteRecoveryCommand(std::string path, bool wipe);
	void ClearRecoveryCommand();
	void WipeData();

	static bool IsRecoverySuccess();
	static bool IsRecoveryFailed();

private:
	static std::string s_szRecoverySuccessPath;
	static std::string s_szRecoveryFailedPath;
};

#endif /* RECOVERYMANAGER_H_ */

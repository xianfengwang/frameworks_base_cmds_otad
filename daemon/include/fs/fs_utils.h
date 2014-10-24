/*
 * fs_utils.h
 *
 *  Created on: 2014-5-20
 *      Author: manson
 */

#ifndef FS_UTILS_H_
#define FS_UTILS_H_

#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

bool fs_IsDirExist(std::string path);
bool fs_IsFileExist(std::string path);
bool fs_MkDir(std::string path);
std::string fs_ReadFile(std::string path);
bool fs_WriteFile(std::string path, std::string content);
void fs_DeleteFile(std::string path);
off_t fs_GetFileSize(std::string path);
void fs_CleanDir(std::string path);

#endif /* FS_UTILS_H_ */

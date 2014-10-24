/*
 * fs_utils.cpp
 *
 *  Created on: 2014-5-20
 *      Author: manson
 */

#include "fs/fs_utils.h"

using namespace std;

bool fs_IsDirExist(string path) {
	DIR *dir = opendir(path.c_str());
	if (dir == NULL) {
		return false;
	}
	return true;
}

bool fs_IsFileExist(string path) {
	return access(path.c_str(), 0) == 0;
}

bool fs_MkDir(string path) {
	if (fs_IsDirExist(path)) {
		return true;
	}
	int res = mkdir(path.c_str(), 0755);
	if (res == 0) {
		return true;
	}
	return false;
}

string fs_ReadFile(string path) {
	ifstream in;
	in.open(path.c_str(), ios::in);
	string line;
	stringstream ss;
	while (!in.eof()) {
		getline(in, line);
		ss << line << endl;
	}
	in.close();
	return ss.str();
}

bool fs_WriteFile(string path, string content) {
	ofstream outfile(path.c_str(), ios::out);
	if (!outfile || !outfile.is_open()) {
		return false;
	}
	outfile << content;
	outfile.flush();
	outfile.close();
	return true;
}

void fs_DeleteFile(string path) {
	remove(path.c_str());
}

off_t fs_GetFileSize(std::string path) {
	struct stat info;
	stat(path.c_str(), &info);
	return info.st_size;
}

void fs_CleanDir(std::string path) {
	stringstream ss;
	ss << "rm -rf ";
	ss << path;
	ss << "*";
	system(ss.str().c_str());
}

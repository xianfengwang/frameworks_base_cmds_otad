/*
 * Copyright (C) 2013-2014, The OpenFlint Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS-IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DEVICEMANAGECLIENT_H_
#define DEVICEMANAGECLIENT_H_

#include "platform/DeviceInfo.h"

using namespace std;

class DeviceManageClient {
public:
	DeviceManageClient();
	virtual ~DeviceManageClient();

	void write_json();
	void clear_json();
	void post_file(const std::string url, const std::string file_path);
private:
	DeviceInfo *m_DeviceInfo;

};

#endif /* DEVICEMANAGECLIENT_H_ */

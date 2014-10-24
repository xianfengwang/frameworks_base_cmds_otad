LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= src/main.cpp \
	src/utils/utils.cpp \
	src/fs/fs_utils.cpp \
	src/platform/PropertyManager.cpp \
	src/platform/DeviceInfo.cpp \
	src/platform/ProcessMonitor.cpp \
	src/download/DownloadTask.cpp \
	src/download/Downloader.cpp \
	src/json/json_reader.cpp \
	src/json/json_value.cpp \
	src/json/json_writer.cpp \
	src/platform/RecoveryManager.cpp \
	src/net/HttpClient.cpp \
	src/fs/md5.cpp \
	src/net/RequestParams.cpp \
	src/net/RequireResult.cpp \
	src/message/OtaMessage.cpp \
	src/fs/mtdutils.c \
	src/message/Message.cpp \
	src/message/SystemMessage.cpp

LOCAL_C_INCLUDES += \
	frameworks/base/cmds/otad/daemon/include \
	frameworks/base/include \
	ndk/sources/cxx_stl/gnu-libstdc++/4.6/libs/armeabi-v7a/include \
	ndk/sources/cxx_stl/gnu-libstdc++/4.6/include \
	frameworks/base/cmds/otad/librecovery

LOCAL_SHARED_LIBRARIES := libcutils liblog libstlport librecovery libcurl

LOCAL_CFLAGS := -DHAVE_CONFIG_H -DANDROID -DEXPAT_RELATIVE_PATH -DALLOW_QUOTED_COOKIE_VALUES -DCOMPONENT_BUILD -DGURL_DLL 

LOCAL_CPPFLAGS += -fexceptions  -fno-rtti

LOCAL_STATIC_LIBRARIES := libsupc++ 

#LOCAL_LDFLAGS += -L$(prebuilt_stdcxx_PATH)/thumb  -lsupc++ 

LOCAL_MODULE:= otad
LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)
LOCAL_MODULE_TAGS := eng
include external/stlport/libstlport.mk #shit here?
include $(BUILD_EXECUTABLE)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE :=JniLibName
LOCAL_SRC_FILES := jniUtil.cpp
include $(BUILD_SHARED_LIBRARY)
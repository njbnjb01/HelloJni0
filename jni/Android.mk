# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)

LOCAL_MODULE    := sel433-jni
LOCAL_SRC_FILES := sel433-jni.c
LOCAL_CERTIFICATE := platform
LOCAL_LDLIBS=-lm -llog
LOCAL_SHARED_LIBRARIES := libutils liblog libcutils

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE    := irscan-jni
LOCAL_SRC_FILES := irscan.c
LOCAL_CERTIFICATE := platform
LOCAL_LDLIBS=-lm -llog

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE    := ds28e01-jni
LOCAL_SRC_FILES := ds28e01.c
LOCAL_CERTIFICATE := platform
LOCAL_LDLIBS=-lm -llog

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE    := iccard-jni
LOCAL_SRC_FILES := iccard.c
LOCAL_CERTIFICATE := platform
LOCAL_LDLIBS=-lm -llog

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE    := rfid-jni
LOCAL_SRC_FILES := rfid.c
LOCAL_CERTIFICATE := platform
LOCAL_LDLIBS=-lm -llog

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE    := cantest-jni
LOCAL_SRC_FILES := cantest.c
LOCAL_CERTIFICATE := platform
LOCAL_LDLIBS=-lm -llog

include $(BUILD_SHARED_LIBRARY)



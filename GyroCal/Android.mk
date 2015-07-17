LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    gyro_cal_daemon.c

LOCAL_CPPFLAGS := \
    -std=gnu++11 \
    -W -Wall -Wextra \
    -Wunused \
    -Werror \

LOCAL_SHARED_LIBRARIES := libc libstlport liblog

LOCAL_CFLAGS += -DLOG_TAG=\"ST_GYRO_CAL_D\"

LOCAL_MODULE := stgyrod

include $(BUILD_EXECUTABLE)


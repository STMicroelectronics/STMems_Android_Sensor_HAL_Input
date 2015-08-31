LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    st_cal_daemon.c

LOCAL_CPPFLAGS := \
    -std=gnu++11 \
    -W -Wall -Wextra \
    -Wunused \
    -Werror \

LOCAL_SHARED_LIBRARIES := libc liblog

LOCAL_CFLAGS += -DLOG_TAG=\"STCalibrationD\"

LOCAL_MODULE := stcald

include $(BUILD_EXECUTABLE)


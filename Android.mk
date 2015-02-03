# Copyright (C) 2015 STMicroelectronics
# Giuseppe Barba, Alberto Marinoni - Motion MEMS Product Div.
# Copyright (C) 2008 The Android Open Source Project
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


ifneq ($(TARGET_SIMULATOR),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

define all-module-under-lib
	$(wildcard $(LOCAL_PATH)/lib/*/module.mk)
endef

define all-dir-under-lib
	$(foreach directory,$(shell cd $(LOCAL_PATH);find lib/ -type d),\
		$(addprefix $(LOCAL_PATH)/,$(directory)))
endef

define all-cpp-source-files
	$(patsubst ./%,%, \
		$(shell cd $(LOCAL_PATH); find . -name "*.cpp"))
endef

define def_macro
	$(foreach d,$1,$(addprefix -D,$(d)))
endef

################################################################################
# Select enabled sensor trough ENABLED_SENSORS macro.                          #
# Valid values for ENABLED_SENSORS are:                                        #
# - LSM330                                                                     #
# - LSM330D                                                                    #
# - LSM330DLC                                                                  #
# - LSM303C                                                                    #
# - LSM303D                                                                    #
# - LSM303DLHC                                                                 #
# - LSM6DS0                                                                    #
# - LSM6DS3                                                                    #
# - LSM9DS0                                                                    #
# - LSM9DS1                                                                    #
# - L3GD20                                                                     #
# - L3GD20H                                                                    #
# - LIS3MDL                                                                    #
# - LPS331AP                                                                   #
# - V_GYRO                                                                     #
#                                                                              #
# Valid values for ENABLED_MODULES:                                            #
# - SENSOR_FUSION                                                              #
# - GEOMAG_FUSION                                                              #
# - MAGCALIB                                                                   #
# - GBIAS                                                                      #
#                                                                              #
# E.g.: to enable LSM6DS0 + LIS3MDL sensor                                     #
#                ENABLED_SENSORS := LSM6DS0 LIS3MDL                            #
################################################################################
ENABLED_SENSORS := LSM9DS0 
ENABLED_MODULES := SENSOR_FUSION GBIAS GEOMAG_FUSION

LOCAL_ARM_MODE := arm
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE := sensors.$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES := 		\
				$(call all-dir-under-lib) \
				$(LOCAL_PATH)/include/ \
				$(LOCAL_PATH)/conf/
LOCAL_STATIC_LIBRARIES :=

include $(call all-module-under-lib)

LOCAL_CFLAGS := 		\
				-DLOG_TAG=\"Sensors\" \
				-DANDROID_VERSION=$(PLATFORM_SDK_VERSION) \
				$(call def_macro, $(ENABLED_SENSORS)) \
				$(call def_macro, $(ENABLED_MODULES))


LOCAL_SRC_FILES := $(call all-cpp-source-files)

LOCAL_SHARED_LIBRARIES := liblog libcutils libutils libdl libc libstlport

include $(BUILD_SHARED_LIBRARY)
include $(call all-makefiles-under,$(LOCAL_PATH))
endif # !TARGET_SIMULATOR

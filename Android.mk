# Copyright 2006-2014 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += external/tinyalsa/include \
                    system/core/include \
		    external/libyuv/files/include
LOCAL_SRC_FILES:= factorytest.c \
                  framebuffer.c \
		  util.c \
		  mipi_hdmi_test.c \
		  memory_test.c \
		  wifi_test.c \
		  emmc_test.c \
		  uart_test.c \
		  tp_test.c \
		  mic_spdif_test.c \
		  usb_test.c \
		  backlights.c \
		  camera_test.c

LOCAL_SHARED_LIBRARIES := libpixelflinger  libtinyalsa liblog  libaudioqueue  libcamerapreview
LOCAL_STATIC_LIBRARIES := libyuv_static #libminui

LOCAL_LDFLAGS += -L$(LOCAL_PATH)/prebuilt
LOCAL_LDLIBS += -lfftw3

LOCAL_MODULE := factory-test

#LOCAL_CFLAGS := -Werror
LOCAL_CFLAGS +=-D_FORTIFY_SOURCE=0

include $(BUILD_EXECUTABLE)

#include $(call all-subdir-makefiles)
#include $(call first-makefiles-under,$(LOCAL_PATH))
#include external/factory-test/prebuilt/Android.mk

# Copyright 2006-2014 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += external/tinyalsa/include \
                    system/core/include 
LOCAL_SRC_FILES:= statfs.c 
LOCAL_SHARED_LIBRARIES := libpixelflinger  libtinyalsa liblog  libaudioqueue
LOCAL_STATIC_LIBRARIES := #libyuv_static
LOCAL_MODULE := statfs
#LOCAL_CFLAGS := -Werror
LOCAL_CFLAGS +=-D_FORTIFY_SOURCE=0
include $(BUILD_EXECUTABLE)


include $(CLEAR_VARS)
LOCAL_C_INCLUDES += 
LOCAL_SRC_FILES:= tp_test.c 
LOCAL_SHARED_LIBRARIES := #libpixelflinger  libtinyalsa liblog  libaudioqueue
LOCAL_STATIC_LIBRARIES := #libyuv_static
LOCAL_MODULE := tp_test
#LOCAL_CFLAGS := -Werror
#LOCAL_CFLAGS +=-D_FORTIFY_SOURCE=0
include $(BUILD_EXECUTABLE)


include $(CLEAR_VARS)
LOCAL_C_INCLUDES += external/tinyalsa/include \
                    system/core/include
LOCAL_SRC_FILES:= mic_spdif_test.c 
LOCAL_SHARED_LIBRARIES := libtinyalsa liblog
LOCAL_STATIC_LIBRARIES := #libyuv_static
LOCAL_MODULE := mic_spdif_test
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += external/tinyalsa/include \
                    system/core/include
LOCAL_SRC_FILES:= fftw.c
LOCAL_LDFLAGS += -Lexternal/factory-test/prebuilt
LOCAL_SHARED_LIBRARIES := 
LOCAL_STATIC_LIBRARIES :=
LOCAL_LDLIBS += -lfftw3
LOCAL_MODULE := fftw
include $(BUILD_EXECUTABLE)


include $(CLEAR_VARS)
LOCAL_C_INCLUDES += external/tinyalsa/include \
                    system/core/include
LOCAL_SRC_FILES:= chn_check.c
LOCAL_LDFLAGS += -Lexternal/factory-test/prebuilt
LOCAL_SHARED_LIBRARIES := libaudioqueue libtinyalsa
LOCAL_STATIC_LIBRARIES :=
LOCAL_LDLIBS += -lfftw3
LOCAL_MODULE := chn_check
include $(BUILD_EXECUTABLE)


include $(CLEAR_VARS)
LOCAL_C_INCLUDES += external/tinyalsa/include \
                    system/core/include
LOCAL_SRC_FILES:= play.c
LOCAL_SHARED_LIBRARIES :=  libtinyalsa
LOCAL_STATIC_LIBRARIES :=
LOCAL_MODULE := play
include $(BUILD_EXECUTABLE)

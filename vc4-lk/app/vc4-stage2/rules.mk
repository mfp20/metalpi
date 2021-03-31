LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_DEPS += \
	lib/partition \
	lib/fs \
	lib/fs/fat32 \
	lib/fs/ext2 \
	lib/debugcommands \
	app/stringtests \
	platform/bcm28xx/dpi \
	app/tests \

MODULE_SRCS += \
	$(LOCAL_DIR)/stage2.c \

include make/module.mk


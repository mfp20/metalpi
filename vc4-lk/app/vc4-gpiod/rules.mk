LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_DEPS += \
	lib/elf \

MODULE_SRCS += \
	$(LOCAL_DIR)/src/gpiod.c \

include make/module.mk


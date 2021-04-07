
export DRV_ARCH := x86
export DRV_KNL_DIR := $(PWD)
export DRV_DIR	   := $(DRV_KNL_DIR)/..

ifneq ($(KERNELRELEASE),)

    export EXTRA_CFLAGS += -I$(DRV_DIR)/include
    export EXTRA_CFLAGS += -I$(DRV_KNL_DIR)/include/
	export EXTRA_CFLAGS += -I$(DRV_DIR)/cpu/$(DRV_ARCH)/
	
	obj-m   := va.o

	include $(DRV_KNL_DIR)/fs/Makefile.mk
    include $(DRV_KNL_DIR)/comm/Makefile.mk
	include $(DRV_KNL_DIR)/mt/Makefile.mk
	include $(DRV_KNL_DIR)/fdd/Makefile.mk
	include $(DRV_KNL_DIR)/board/Makefile.mk
	include $(DRV_KNL_DIR)/chan/Makefile.mk

	va-objs += $(mt-objs)
	va-objs += $(fs-objs)
	va-objs += $(comm-objs)
	va-objs += $(fdd-objs)
	va-objs += $(board-objs)
	va-objs += $(chan-objs)
else

#	export EXTRA_CFLAGS := -I$(DRV_DIR)/include
#	export EXTRA_CFLAGS += -I$(DRV_KNL_DIR)/include/
#	export EXTRA_CFLAGS += -I$(DRV_DIR)/this_cpu/$(DRV_ARCH)/

	KVERSION = $(shell uname -r)

	PWD = $(shell pwd)

all:
	echo $(EXTRA_CFLAGS)
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean
endif

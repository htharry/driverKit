FDD = ./fdd

fdd-objs := $(FDD)/fdd.o $(FDD)/fdd_buf.o $(FDD)/fdd_mt.o $(FDD)/fdd_port.o

include $(DRV_KNL_DIR)/fdd/vc/Makefile.mk
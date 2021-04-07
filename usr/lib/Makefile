export DRV_ARCH := x86

all : libva

INCLUDE = -I ../../include -I ../include -I ../../cpu/$(DRV_ARCH)/

OBJ = mt_usr.o mt_usr_compat.o vaa_api.o vaa_media_api.o vaa_cfg_api.o

pre:
	rm -f *.o

%.o : %.c
	gcc $(INCLUDE)  -g  -c $<  -o $@ -Wall

libva: pre $(OBJ)
	ar rcs  -o $@.a $(OBJ)

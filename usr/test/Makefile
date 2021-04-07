export DRV_ARCH := x86

INCLUDE = -I ../../include -I ../include -I ../../cpu/$(DRV_ARCH)/

all : pre bin
	echo "complete!"

bin : mt_test mt_compat_test mt_video_test

pre:
	rm -f *.o

%.o : %.c
	gcc $(INCLUDE)  -g  -c $<  -o $@ -Wall

mt_test:mt_test.o
	gcc -o $@ $< ../lib/libva.a

mt_compat_test:mt_compat_test.o
	gcc -o $@ $< ../lib/libva.a

mt_video_test:mt_video_test.o
	 gcc -o $@ $< ../lib/libva.a -lpthread



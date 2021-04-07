export DRV_ARCH := x86

all : mt

INCLUDE = -I ../../include -I ../include -I ../../cpu/$(DRV_ARCH)/

OBJ = mt_input.o mt_main.o mt_parse.o mt_io_cmd.o mt_util.o

%.o : %.cpp
	g++ $(INCLUDE)  -g  -c $<  -o $@
mt:$(OBJ)
	g++  -o $@ $(OBJ)

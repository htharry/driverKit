all:
	make -C ./kern 
	make -C ./usr/lib all
	make -C ./usr/mt all
	make -C ./usr/test all

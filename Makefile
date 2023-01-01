CONFIG_MODULE_SIG=n

obj-m += modreveal.o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement
PWD := $(CURDIR)/build

all:
	mkdir -p build
	cp $(CURDIR)/Makefile $(CURDIR)/build/.
	cp $(CURDIR)/kernel/modreveal.c $(CURDIR)/build/.
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc $(CURDIR)/user/modreveal.c -I/usr/include/libnl3 -lnl-3 -lnl-genl-3 -o $(CURDIR)/build/modreveal
	cp $(CURDIR)/build/modreveal $(CURDIR)/.
	cp $(CURDIR)/build/modreveal.ko $(CURDIR)/.

clean:
	-make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	-rm -rf build
	-rm modreveal
	-rm modreveal.ko

cycle: clean all
	-sudo rmmod modreveal
	sudo insmod ./build/modreveal.ko


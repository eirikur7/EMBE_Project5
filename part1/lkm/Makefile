obj-m += gpio_copy.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean

response_test: response_test.c
	gcc -Wall -o response_test response_test.c -lgpiod
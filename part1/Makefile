all: gpio_poll gpio_events

gpio_poll: gpio_poll.c
	gcc -Wall -o gpio_poll gpio_poll.c -lgpiod

gpio_events: gpio_events.c
	gcc -Wall -o gpio_events gpio_events.c -lgpiod

clean:
	rm -f gpio_poll gpio_events

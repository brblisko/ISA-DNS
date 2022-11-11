CFLAGS=-std=gnu99 -Wall -Wextra -pedantic -pthread -g # -Werror
RFILES=./receiver/main.c ./receiver/dns_receiver_events.c ./receiver/parameter_parser.c
SFILES=./sender/main.c ./sender/dns_sender_events.c ./sender/parameter_parser.c

.PHONY: all receiver sender clean

all : receiver sender

receiver : $(RFILES)
	gcc $(CFLAGS) -o dns_receiver $(RFILES)

sender : $(SFILES)
	gcc $(CFLAGS) -o dns_sender $(SFILES)

clean :
	rm -f *.o dns_sender dns_receiver
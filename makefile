all:
	gcc msgsaucer.c -lpthread -o messenger.out
	gcc msgsaucer_send.c -o sauce.out
clean:
	rm *.out

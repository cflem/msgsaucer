all:
	gcc saucer.c -lpthread -o saucer
	gcc sauce.c -o sauce
clean:
	rm -f *.out saucer sauce

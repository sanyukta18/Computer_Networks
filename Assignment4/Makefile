MRP_Users 	:	librsocket.a
	gcc -o user2 user2.c -L. -lrsocket -lpthread
	gcc -o user1 user1.c -L. -lrsocket -lpthread

librsocket.a	:	rsocket
	ar rcs librsocket.a rsocket

rsocket	:	rsocket.c	rsocket.h
	gcc rsocket.h 
	gcc -c -o rsocket rsocket.c

clean	:	
	rm -f user2 user1 librsocket.a rsocket rsocket.h.gch
	clear

remove	:	
	rm -f user2 user1 librsocket.a rsocket rsocket.h.gch
    
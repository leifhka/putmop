
compile:
	gcc -std=c99 -Wall -o putmop putmop.c -lcurses
run: 
	./putmop

clean:
	rm -v ./putmop


compile:
	gcc -Wall -o putmop putmop.c -lcurses
run: 
	./putmop

clean:
	rm -v ./putmop

all: myshell

myshell:
	gcc -o myshell shell2.c
clean:
	rm myshell
all: shell

shell:
	gcc -o shell shell2.c
clean:
	rm -f shell

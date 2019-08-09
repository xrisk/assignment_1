all: reverse verify

reverse: reverse.c
	gcc -Ofast -m64 -march=native reverse.c -o reverse

verify: reverse.c
	gcc -Ofast -m64 -march=native verify.c -o reverse

clean:
	rm verify reverse

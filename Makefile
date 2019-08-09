all: reverse verify

reverse: reverse.c
	gcc -Ofast -m64 -march=native reverse.c -o reverse

verify: verify.c
	gcc -Ofast -m64 -march=native verify.c -o verify

clean:
	rm -rf verify reverse Assignment/

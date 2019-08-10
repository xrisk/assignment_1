all: Q1 Q2

Q1: Q1.c
	gcc -Ofast -m64 -march=native Q1.c -o Q1

Q2: Q2.c
	gcc -Ofast -m64 -march=native Q2.c -o Q2

clean:
	rm -rf Q1 Q2 Assignment/

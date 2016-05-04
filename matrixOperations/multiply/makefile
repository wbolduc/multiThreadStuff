All: matrix matrixMultiply
	gcc -g -o mm matrix.o matrixMultiply.o

matrixMultiply: matrix
	gcc -g -pthread -c matrixMultiply.c
	
matrix:
	gcc -g -c matrix.c



clean:
	rm *.o
	rm mm

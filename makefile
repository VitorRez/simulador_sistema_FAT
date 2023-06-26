# _*_Makefile_*_ 
tp3: tp3.o fat.o
	gcc -o tp3 tp3.o fat.o

fat.o: fat.c fat.h
	gcc -c fat.c

tp3.o: tp3.c fat.h
	gcc -c tp3.c

clean:
	rm *.o tp3
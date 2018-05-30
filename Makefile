# https://stackoverflow.com/questions/1484817/how-do-i-make-a-simple-makefile-for-gcc-on-linux

default: bin/quartex

bin/quartex: quartex.c
	gcc -g quartex.c -o bin/quartex

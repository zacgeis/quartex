# https://stackoverflow.com/questions/1484817/how-do-i-make-a-simple-makefile-for-gcc-on-linux

default: quartex

VMTranslator: quartex.c
	gcc -g quartex.c -o quartex


#ifndef STRIN_H
#define STRIN_H

#include <stdlib.h>
#include <stdio.h>

#define STRIN_BUCKET 50

typedef enum{OK, FAIL} error_type;

typedef struct{
	size_t msize;
	int slen;
	char* s;
} Strin;


Strin* newStrin(); //crea un nuevo string con 0 caracteres

int addcToStrin(Strin* s,char c); //agrega un nuevo caracter al string

char* retChar(Strin* s); //retorna un char* para poder manipularlo
#endif

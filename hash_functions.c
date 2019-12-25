#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"

char *hash(FILE *f){
	int i = 0;
	char *hash = (char *) malloc(BLOCK_SIZE * sizeof(char));
	for (; i < BLOCK_SIZE; i++){
		hash[i] = '0';
	}
	char input;
	while ( fread(&input, 1, 1, f) != 0){
		hash[i%BLOCK_SIZE] = hash[i%BLOCK_SIZE] ^ input;
		i++; 	
	}
	return hash;
}


#include <stdio.h>
#include <stdlib.h>
/*
 * Jake McKenzie
 * 
 * This file reads two file names and a variable buffer size.
 * 
 * Instructions
 * 
 * gcc file_reader_writer.c <file to be read> <file to be written> <variable buffer size>
 */	
int main(int argc,char **argv) {
	//bytes to be read and written
	char c;
	//variable buffer size
	int bs = atoi(argv[3]);
	//the two file pointers
	c = bs
	FILE *fp1, *fp2;
	fp1 = fopen(argv[1], "r");
	
	
	if( fp1 == NULL ) {
		printf("\n The file failed to read. \n");
		exit(EXIT_FAILURE);
	}
	
	fp2 = fopen(argv[2], "w");
	
	if( fp2 == NULL ) {
		fclose(fp1);
		printf("The file cannot be written to. \n");
		exit(EXIT_FAILURE);
	}
		
	char *buffer = (char*) malloc(bs);
	
	while( ( c = fgetc(fp1) ) != EOF ){	
		fputc(c, fp2);
	}
	
	printf("File provided was read correctly and file output was written to successfully!\n");

	fclose(fp1);
	fclose(fp2);
	
	return 0;
}

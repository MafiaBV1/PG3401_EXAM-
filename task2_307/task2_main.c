/*
 * PG3401 EXAM 2026 
 * Task 2: File management and functions (15%) 
 * 
 * Reads integers from pgexam26_test.txt, evaluates each number 
 * using the provided functions. Then it writes the results in BINARY 
 * to pgexam26_output.bin using the TASK"_NUMBERS_METADATA struct 
 */

#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include "task2.h"

int main (void) {
	const char *input_file = "pgexam26_test.txt"; 
	const char *output_file = "pgexam26_output.bin"; 
	FILE *fin; 
	FILE *fout; 
	struct TASK2_NUMBERS_METADATA meta; 
	int number; 
	int index; 
	
	fin = fopen(input_file, "r"); 
	if (fin == NULL) {
		fprintf(stderr, "Errpr: Could not open input file '%s'\n", input_file); 
		return 1; 
	}
	
	fout = fopen(output_file, "wb"); 
	if (fout == NULL) {
		fprintf(stderr, "Error: Could not open output file '%s'\n",output_file); 
		return 1; 
	}
	
	index = 0; /* index set at 0 not 1 */
	printf(" --- Running task2 ----\n"); 
	printf("Reading from '%s', writing binary too '%s\n\n", input_file, output_file); 
	printf("%-5s %-10s %-5s %-5s %-6s %-5s %-7s %-8s %-8s %-5s\n",
			"Idx", "Number", "Fib", "Prime", "Sq", "Cude", "Perf", "Abun", "Def", "Odd"); 
	printf("-----------------------------------------------------------------------\n");
	
	while (fscanf(fin, "%d", &number) == 1) {
		memset(&meta, 0, sizeof(meta)); 
		meta.iIndex				= index; 
		meta.iNumber			= number; 
		meta.bIsFibonacci 		= (int)isFibonacci(number); 
		meta.bIsPrimeNumber 	= (int)isPrime(number); 
		meta.bIsSquareNumber	= (int)isSquareNumber(number); 
		meta.bIsCubeNumber		= (int)isCubeNumber(number); 
		meta.bIsPerfectNumber	= (int)isPerfectNumber(number); 
		meta.bIsAbundantNumber	= (int)isAbundantNumber(number); 
		meta.bIsDeficientNumber	= (int)isDeficientNumber(number); 
		meta.bIsOddNumber		= (int)isOdd(number); 
		printf("%-5d %-10d %-5d %-5d %-6d %-5d %-7d %-8d %-8d%-5d\n",
				meta.iIndex, meta.iNumber, meta.bIsFibonacci, 
				meta.bIsPrimeNumber, meta.bIsSquareNumber, meta.bIsCubeNumber, 
				meta.bIsPerfectNumber, meta.bIsAbundantNumber, 
				meta.bIsDeficientNumber, meta.bIsOddNumber); 
		
		/* Write struct as binary */ 
		fwrite(&meta, sizeof(struct TASK2_NUMBERS_METADATA), 1, fout); 
		index++; 
	}
	fclose(fin); 
	fclose(fout); 
	printf("\nDone! Wrote %d records to '%s'\n", index -1, output_file); 
	return 0; 
}

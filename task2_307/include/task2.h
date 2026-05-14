#ifndef TASK2_H
#define TASK2_H 

#include <stdbool.h> 
#include <math.h> 

/* Function declarations from EWA-generated source files */
bool isFibonacci(int n); 
bool isPrime(int n); 
bool isSquareNumber(int n); 
bool isCubeNumber(int n); 
bool isPerfectNumber(int n); 
bool isAbundantNumber(int n); 
bool isDeficientNumber(int n); 
bool isOdd(int n); 



/* Struct as specified in exam task */
struct TASK2_NUMBERS_METADATA {
	int iIndex;  	/* Sequence number, where first number = 1 */
	int iNumber;	/* The number itself, as read from file */	
	int bIsFibonacci;
	int bIsPrimeNumber; 
	int bIsSquareNumber; 
	int bIsCubeNumber;
	int bIsPerfectNumber;
	int bIsAbundantNumber;
	int bIsDeficientNumber; 
	int bIsOddNumber; 			
}; 

#endif /* TASK2_H */

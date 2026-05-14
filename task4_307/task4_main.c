/* This file has been created by EWA, and is part of task 4 on the exam for PG3401 2026*/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "task4_prime.h" /* Adding the header file */

/* Struct to hold all shared data passed to each thread as a parameter
 * Previously pPrimeList and fInputFile were global variables, which is 
 * unsafe in a multithreaded program. By passing them as a struct pointer, 
 * all threads share the same data through a single controlled reference 
 */
struct THREADARGS {
	struct PGPRIMENUMBER *pPrimeList; 
	FILE *fInputFile;
	pthread_mutex_t *pMutexFile; /* Protects fInputFile reads */
	pthread_mutex_t *pMutexList: /* Protects pPrimeList writes */
}; 

struct PGPRIMENUMBER {
   unsigned int uiPrime;
   struct PGPRIMENUMBER* next;
};

void* threadFunction(void* arg){
	/*
	* Receive shared data through parameter insted of global veriables
	* Without this, both theads would access pPrimeList and fInputFile
	* directly as global with no synchronization, causing data races
	*/
  	unsigned int auiNumbers[10] = {0};
   	int iNumbersRead = 0;
   	int iIndex = 0;
   	struct PGPRIMENUMBER* newNode = NULL;

   while (true) {
      iNumbersRead = 0;
      while (iNumbersRead < 10) {
         if (fscanf(fInputFile, "%d", &(auiNumbers[iNumbersRead])) == 1) {
            iNumbersRead++;
            /*
	 		* MUTEX LOCK - file read 
	 		* Problem without mutex: Both threads call fscanf on the same FILE* 
	 		* simultaneously. fscanf is not thread-safe; concurrent reads corrupt 
	 		* the file position pointer and cause numbers to be skipped or read twice
	 		* Solution: Lock the mutex before reading so only one thread reads at a time 
	 		*/
         } else {
         	break;
         }
      }
      
      if (iNumbersRead == 0) {
         break;
      }

      for (iIndex = 0; iIndex < iNumbersRead; iIndex++) {
         if (isPrime(auiNumbers[iIndex])) {
            newNode = (struct PGPRIMENUMBER*)malloc(sizeof(struct PGPRIMENUMBER));
            if (newNode != NULL) {
            	newNode->uiPrime = auiNumbers[iIndex]; 
            	/*
				* MUTEX LOCK - linked list write
				* Problem without mutex: Both threads may find a prime at the 
				* same time and both execute newNode->next = pPrimeList followed
				* by pPrimeList = newNode. This is a classic race condition where 
				* one node overwrites the other, causing memory leaks and a 
				* corrupted list. 
				* Solution: Lock the mutex so only one thread modifies the 
				* linked list at a time, ensuring all primes are inserted safelt
				*/
				pthread_mutex_lock(pArgs->pMutexList); 
				newNode->next = pArgs->pPrimeList; 
				pArgs->pPrimeList = newNode; 
				newNode = NULL; 
				pthread_mutex_unlock(pArgs->pMutexList); 
				/* MUTEX UNLOCK - linked list wite */
            }
         }
      }
   }

   return NULL;
}
	 
int main(int argc, char* argv[]) {
	pthread_t thread1; 
	pthread_t thread2; 
	struct PGPRIMENUMBER *pPter = NULL; 
	int rc = 0; 
	struct THREADARGS tArgs; 
	pthread_mutex_t mutexFile; 
	pthread_mutex_t mutexList; 
	
	/* Validate command line argument */ 
	if (argc < 2) {
		printf("Usage: %s <filename>\n", argv[0]);
		return 1; 
	}
	
	/* 
	* Explicit mutex initialization using *_init functions as reqired
	* without explicit initialization the mutex state is undefined
	* pthread_mutex_init() sets up the mutex with default attributes (NULL),
	* making it ready for use before any threadtries to lock it 
	*/
	pthread_mutex_init(&mutexFile, NULL); 
	pthread_mutex_init(&mutexList, NULL); 
	
	/* Iniialize shared thread arguments - no global variables used */
	tArgs.pPrimeList = NULL; 
	tArgs.fInputFile = NULL; 
	tArgs.pMutexFile = &mutexFile; 
	tArgs.pMutexList = &mutexList; 
	
	/* Open file from command line argument insted of hardcoded filename */
	tArgs.fInputFile = fopen(argv[1], "r"); 
	if (tArgs.fInputFile == NULL) {
		printf("Error: Could not open file '%s'\n", argv[1]); 
		pthread_mutex_destroy(&mutexFile); 
		pthread_mutex_destroy(&mutexList); 
		return 1; 
	} 
	
	rc = pthread_create(&thread1, NULL, threadFunction, (void*)&tArgs); 
	if (rc != 0) {
		fclose(tArge.fInputFile); 
		pthread_mutex_destroy(&mutexFile); 
		pthread_mutex_destroy(&mutexList); 
		return 1; 
	}
	
	rc = pthread_create(&thread2, NULL, threadFunction, (void*)&tArgs); 
	if (rc != 0) {
		pthread_join(thread1, NULL); 
		fclose(tArge.fInputFile); 
		pthread_mutex_destroy(&mutexFile); 
		pthread_mutex_destroy(&mutexList); 
		return 0; 
	}
	
	pthread_join(thread1, NULL); 
	pthread_join(thread2, NULL); 
	
	fclose(tArgs.fInputFile); 
	
	printf("\r\nPrime numbers found : \r\n"); 
	pPtr = tArgs.pPrimeList; 
	while (pPtr != NULL) {
		printf("%d\r\n", pPtr->uiPrime); 
		pPtr = pPtr>next; 
	}
	
	/* Free linked list */ 
	while (tArgs.pPrimeList != NULL) {
		pPtr = tArgs.pPrimeList; 
		tArgs.pPrimeList = tArgs.pPrimeList->next; 
		free(pPtr); 
		pPtr = NULL; 
	}
	
	/* Destroy mutexes when no longer needed */ 
	pthread_mutex_destroy(&mutexFile); 
	pthread_mutex_destroy(&mutexList); 
	return 0; 
}	 

/* This file has been created by EWA, and is part of task 4 on the exam for PG3401 2026*/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "task4_prime.h" /* Adding the header file */

void tean(unsigned int *const v, unsigned int *const w, const unsigned int *const k, int N); 
unsigned int tcp_checksum(const unsigned char *data, size_t lenght);

struct PGPRIMENUMBER {
	unsigned int uiPrime;
   	struct PGPRIMENUMBER *next;
};

 /* Struct to hold all shared data passed to each thread as a parameter
 * Previously pPrimeList and fInputFile were global variables, which is 
 * unsafe in a multithreaded program. By passing them as a struct pointer, 
 * all threads share the same data through a single controlled reference 
 */
struct THREADARGS {
	struct PGPRIMENUMBER *pPrimeList; 
	FILE 				 *fInputFile;
	pthread_mutex_t 	 *pMutexFile; /* Protects fInputFile reads */
	pthread_mutex_t 	 *pMutexList; /* Protects pPrimeList writes */
	
	/* Part II additions */ 
	unsigned char 		 *pCodeBuf;		/* Encrypted file buffer */
	size_t 				 iCodeSize; 	/* Size of encrypted file */
	int 				 *pFound;		/* Flag: set to 1 when key found */ 
	pthread_mutex_t 	 *pMutexFound;	/* Protects oFound flag */ 
}; 


void* threadFunction(void* arg) {
	/*
	* Receive shared data through parameter insted of global veriables
	* Without this, both theads would access pPrimeList and fInputFile
	* directly as global with no synchronization, causing data races
	*/
	struct THREADARGS		*pArgs 			= (struct THREADARGS*)arg; 
  	unsigned int 			auiNumbers[10]; 
  	int 					iNumbersRead 	= 0; 
  	int 					iIndex 			= 0; 
  	struct PGPRIMENUMBER	*newNode 		= NULL;
  	unsigned int 			key[4]; 
  	unsigned char 			*decBuf			= NULL; 
  	size_t					iBlock			= 0; 
  	unsigned int 			v[2]; 
  	unsigned int 			w[2]; 
  	FILE 					*fOut			= NULL; 
  	unsigned int 			checksum		= 0; 
  	
  	while (1) {
  		iNumbersRead = 0; 
  		/*
		 * MUTEX LOCK - file read 
		 * Problem without mutex: Both threads call fscanf on the same FILE* 
		 * simultaneously. fscanf is not thread-safe; concurrent reads corrupt 
		 * the file position pointer and cause numbers to be skipped or read twice
		 * Solution: Lock the mutex before reading so only one thread reads at a time 
		 */
		pthread_mutex_lock(pArgs->pMutexFile); 
		
		/* Check if another thread already found the key, stop reading if so */ 
		if (*(pArgs->pFound)) {
			pthread_mutex_unlock(pArgs->pMutexFile); 
			break; 
		}
		
		while (iNumbersRead < 10) {
			if (fscanf(pArgs->fInputFile, "%u", &(auiNumbers[iNumbersRead])) == 1) {
				iNumbersRead++; 
			} else {
				break; 
			}
		}
		pthread_mutex_unlock(pArgs->pMutexFile);
		 
		/* MUTEX UNLOCK file read */ 
		if (iNumbersRead == 0) { break; }
		for (iIndex = 0; iIndex < iNumbersRead; iIndex++) {
		
			/* Check if key is already found by other thread's and quit */
			pthread_mutex_lock(pArgs->pMutexFile); 
			if (*(pArgs->pFound)) {
				pthread_mutex_unlock(pArgs->pMutexFile); 
				return NULL; 
			}
			pthread_mutex_unlock(pArgs->pMutexFile); 
			
			if (isPrime(auiNumbers[iIndex])) {
			printf("Prime found: %u\n", auiNumbers[iIndex]); 
				newNode = (struct PGPRIMENUMBER*)malloc(sizeof(struct PGPRIMENUMBER)); 
				if (newNode != NULL) {
					newNode->uiPrime = auiNumbers[iIndex]; 
					/* 
					* MUTEX LOCK linked list write 
					* Problem without mutex: Both threads may find a prime at the 
					* same time and both execute newNode->next = pPrimeList followed
					* by pPrimeList = newNode. This is a race condition where 
					* one node overwrites the other, causeing memory leaks and a 
					* corrupted list
					* Solution: Lock mutex so only one thread modifies the 
					* linked kist at a time, ensureing all primes are inserted safely 
					*/
					pthread_mutex_lock(pArgs->pMutexList); 
					newNode->next 	= pArgs->pPrimeList; 
					pArgs->pPrimeList  = newNode; 
					newNode 		= NULL; 
					pthread_mutex_unlock(pArgs->pMutexList); 
					/* MUTEX UNLOCK linked list weite */ 
				}
				
				/* PART II try this prime as XTEA key */ 
				key[0] = auiNumbers[iIndex]; 
				key[1] = auiNumbers[iIndex]; 
				key[2] = auiNumbers[iIndex]; 
				key[3] = auiNumbers[iIndex]; 
				
				decBuf = (unsigned char*)malloc(pArgs->iCodeSize); 
				if (decBuf == NULL) {
					continue; 
				}
				/* Decrypt block by block (8 bytes = 2 unsigned ints per block) */ 
				for (iBlock = 0; iBlock + 8 <= pArgs->iCodeSize; iBlock += 8) {
					memcpy(v, pArgs->pCodeBuf + iBlock, 8); 
					tean(v, w, key, -32); 
					memcpy(decBuf + iBlock, w, 8); 
				}
				
				/* Check if first 5 characters are BENGT */ 
				if (memcmp(decBuf, "BENGT", 5) == 0) {
				   /* 
					* MUTEX LOCK found flag 
					* Signal other threads to stop as soon as possible 
					*/
					pthread_mutex_lock(pArgs->pMutexFound);  
					*(pArgs->pFound) = 1; 
					pthread_mutex_unlock(pArgs->pMutexFound); 
					
					printf("Key found: %u\n", auiNumbers[iIndex]); 
					printf("Decrypted: %s\n", decBuf); 
					
					
					/* Save decrypted file */ 
					fOut = fopen("task4_plain.txt", "wb"); 
					if (fOut != NULL) {
						fwrite(decBuf, 1, pArgs->iCodeSize, fOut); 
						fclose(fOut); 
					}
					
					/* Calculate and save IP checksum */ 
					checksum = tcp_checksum(decBuf, pArgs->iCodeSize); 
					fOut = fopen("task4_plain.hash", "w"); 
					if (fOut != NULL) {
						fprintf(fOut, "%u\n", checksum); 
						fclose(fOut); 
					}
					
					free(decBuf); 
					return NULL; 
				}
				free(decBuf); 
				decBuf = NULL; 
			}
		}
	}
	return NULL;
}
			 
int main(int argc, char* argv[]) {
	pthread_t 			 thread1; 
	pthread_t 			 thread2; 
	struct PGPRIMENUMBER *pPtr = NULL; 
	int rc = 0; 
	struct THREADARGS 	 tArgs; 
	pthread_mutex_t 	 mutexFile; 
	pthread_mutex_t 	 mutexList;
	pthread_mutex_t 	 mutexFound;
	 
	/* PART II */ 
	int iFound 		 	 = 0; 
	FILE *fCode 		 = NULL; 
	unsigned char *pCodeBuf = NULL; 
	size_t iCodeSize 	 = 0; 
	
	/* Validate command line argument */ 
	if (argc < 2) {
		printf("Usage: %s <filename>\n", argv[0]);
		return 1; 
	}
	
	printf("--- Running task4 ---\n"); 
	printf("checking for prime numbers...\n"); 
	
	/* 
	* Explicit mutex initialization using *_init functions as reqired
	* without explicit initialization the mutex state is undefined
	* pthread_mutex_init() sets up the mutex with default attributes (NULL),
	* making it ready for use before any threadtries to lock it 
	*/
	pthread_mutex_init(&mutexFile,  NULL); 
	pthread_mutex_init(&mutexList,  NULL); 
	pthread_mutex_init(&mutexFound, NULL);
	
	/* Load encrypted file into memory */ 
	fCode = fopen("task4_code.bin", "rb"); 
	if (fCode != NULL) {
		fseek(fCode, 0, SEEK_END); 
		iCodeSize = ftell(fCode); 
		rewind(fCode); 
		pCodeBuf = (unsigned char*)malloc(iCodeSize);
		if (pCodeBuf != NULL) {
			fread(pCodeBuf, 1, iCodeSize, fCode); 
		} 
		fclose(fCode); 
	}
	
	/* Initialize shared thread arguments, no globalvariables used */ 
	tArgs.pPrimeList  = NULL; 
	tArgs.fInputFile  = NULL;
	tArgs.pMutexFile  = &mutexFile;
	tArgs.pMutexList  = &mutexList; 
	tArgs.pCodeBuf    = pCodeBuf; 
	tArgs.iCodeSize   = iCodeSize; 
	tArgs.pFound      = &iFound; 
	tArgs.pMutexFound = &mutexFound; 
	
	/* Open file from command line argument insted of hardcoeded filename */ 
	tArgs.fInputFile = fopen(argv[1], "r"); 
	if (tArgs.fInputFile == NULL) {
		printf("Error: Could not open file '%s'\n", argv[1]); 
		pthread_mutex_destroy(&mutexFile);
		pthread_mutex_destroy(&mutexList);
		pthread_mutex_destroy(&mutexFound);
		//free(pCodeBuf); 
		return 1;  
	}
	
	rc = pthread_create(&thread1, NULL, threadFunction, (void*)&tArgs); 
	if (rc != 0) {
		fclose(tArgs.fInputFile); 
		pthread_mutex_destroy(&mutexFile); 
		pthread_mutex_destroy(&mutexList);
		pthread_mutex_destroy(&mutexFound); 
		return 1; 
	}
	
	rc = pthread_create(&thread2, NULL, threadFunction, (void*)&tArgs); 
	if (rc != 0) {
		pthread_join(thread1, NULL); 
		fclose(tArgs.fInputFile); 
		pthread_mutex_destroy(&mutexFile); 
		pthread_mutex_destroy(&mutexList);
		pthread_mutex_destroy(&mutexFound); 
		return 0; 
	}
	
	pthread_join(thread1, NULL); 
	pthread_join(thread2, NULL); 
	free(pCodeBuf);
	fclose(tArgs.fInputFile); 
	
	/*
	printf("\r\nPrime numbers found: \r\n"); 
	pPtr = tArgs.pPrimeList; 
	while (pPtr != NULL) {
		printf("%d\r\n", pPtr->uiPrime); 
		pPtr = pPtr->next; 
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
	pthread_mutex_destroy(&mutexFound); 	
	return 0; 
}



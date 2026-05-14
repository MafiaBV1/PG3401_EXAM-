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

struct PGPRIMENUMBER* pPrimeList = NULL;
FILE* fInputFile = NULL;

void* threadFunction(void* arg){
  	unsigned int auiNumbers[10] = {0};
   	int iNumbersRead = 0;
   	int iIndex = 0;
   	struct PGPRIMENUMBER* newNode = NULL;

   while (true) {
      iNumbersRead = 0;
      while (iNumbersRead < 10) {
         if (fscanf(fInputFile, "%d", &(auiNumbers[iNumbersRead])) == 1) {
            iNumbersRead++;
         }
         else {
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
               newNode->next = pPrimeList;
               pPrimeList = newNode; newNode = NULL;
            }
         }
      }
   }

   return NULL;
}

int main(int argc, char* argv[]){
   pthread_t thread1;
   pthread_t thread2;

   struct PGPRIMENUMBER* pPtr = NULL;
   int rc;

   fInputFile = fopen("task4_primes.txt", "r");
   if (fInputFile == NULL) {
      return 1;
   }

rc = pthread_create(&thread1, NULL, threadFunction, (void*)1L);
   if (rc != 0) {
      fclose(fInputFile);
      return 1;
   }
   rc = pthread_create(&thread2, NULL, threadFunction, (void*)2L);
   if (rc != 0) {
      pthread_join(thread1, NULL);
      fclose(fInputFile);
      return 0;
   }

   pthread_join(thread1, NULL);
   pthread_join(thread2, NULL);

   fclose(fInputFile);

   printf("\r\nPrime numbers found : \r\n");
   pPtr = pPrimeList;
   while (pPtr != NULL) {
      printf("%d\r\n", pPtr->uiPrime);
      pPtr = pPtr->next;
   }

   while (pPrimeList != NULL) {
      pPtr = pPrimeList;
      pPrimeList = pPrimeList->next;
      free(pPtr); pPtr = NULL;
   }

   return 0;
}

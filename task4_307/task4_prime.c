/* This file has been created by EWA, and is part of task 4 on the exam for PG3401 2026*/
/* Do not change the implementation of the isPrime function*/
#include <stdio.h>
#include "task4_prime.h"
int isPrime(unsigned int n) {
   unsigned int i = 0;
   /* printf("%i: starting...", (int)time(NULL)); */
   for (i = 2; i < n; i++) {
      if (n % i == 0) {
         break;
      }
   }
   if (i < n) {
      /* printf("%i: %i not a prime...", (int)time(NULL), n); */
   }
   else {
      /* printf("%i: %i IS prime...", (int)time(NULL), n); */
      return n;
   }
   /* printf("%i: completed

", (int)time(NULL)); */
   return 0;
}

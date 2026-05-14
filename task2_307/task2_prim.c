/* This file has been created by EWA, and is part of task 2 on the exam for PG3401 2026*/
#include <stdbool.h>
bool isPrime(int n) {
   int i = 0;
   if (n <= 1) return false;
   for (i = 2; i < n; i++) {
      if (n % i == 0) {
         return false;
      }
   }
   return true;
}

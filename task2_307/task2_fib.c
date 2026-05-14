/* This file has been created by EWA, and is part of task 2 on the exam for PG3401 2026*/
#include <stdbool.h>
bool isFibonacci(int n) {
   int a = 0, b = 1, c = a + b;
   while (c < n) {
      a = b;
      b = c;
      c = a + b;
   }
   return (c == n);
}

/* This file has been created by EWA, and is part of task 2 on the exam for PG3401 2026*/
#include <stdbool.h>
#include <math.h>
bool isSquareNumber(int n) {
   int sqrt_n = (int)sqrt(n);
   return (sqrt_n * sqrt_n == n);
}

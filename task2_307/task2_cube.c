/* This file has been created by EWA, and is part of task 2 on the exam for PG3401 2026*/
#include <stdbool.h>
#include <math.h>
bool isCubeNumber(int n) {
   int cube_n = (int)cbrt(n);
   return (cube_n * cube_n * cube_n == n);
}

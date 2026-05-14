/* This file has been created by EWA, and is part of task 4 on the exam for PG3401 2026*/
/* Original code credited to David Wheeler and Roger Needham, here refernced as XTEA or TEAN */
/* N should be set to 32 cycles, and k should be an array of 4 int (for this task the 4 integers are the same) */
/* The same function is used for both encrypt and decrypt */
/* Documentation: https://www.cix.co.uk/~klockstone/xtea.pdf */ 
void tean(unsigned int *const v,unsigned int *const w,const unsigned int *const k, int N){
   unsigned int y = v[0], z = v[1], DELTA = 0x9e3779b9;
   unsigned int limit = 0, sum = 0;
   if (N > 0) { /* coding */
      limit = DELTA * N; sum = 0;
      while (sum != limit) {
         y += (z << 4 ^ z >> 5) + z ^ sum + k[sum & 3];
         sum += DELTA;
         z += (y << 4 ^ y >> 5) + y ^ sum + k[sum >> 11 & 3];
      }
   }
   else { /* decoding */
      sum = DELTA * (-N);
      while (sum) {
         z -= (y << 4 ^ y >> 5) + y ^ sum + k[sum >> 11 & 3];
         sum -= DELTA;
         y -= (z << 4 ^ z >> 5) + z ^ sum + k[sum & 3];
      }
   }
   w[0] = y; w[1] = z;
}

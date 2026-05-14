/* This file has been created by EWA, and is part of task 2 on the exam for PG3401 2026*/
/* Code based off definitions in RFC 791 (IPv4) and RFC 793 (TCP), which are in the public domain */
unsigned int tcp_checksum(const unsigned char * data, size_t length) { 
   unsigned int sum = 0; 
   for (size_t i = 0; i < length; i += 2) { 
      short word = data[i] << 8; 
      if (i + 1 < length) { 
         word |= data[i + 1]; 
      } 
      sum += word; 
      if (sum > 0xFFFF) { 
         sum = (sum & 0xFFFF) + 1; 
      } 
   } 
   return ~sum; 
}

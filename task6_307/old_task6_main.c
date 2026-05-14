/*
 * Task6: EWP-over-TCP newtwork client 
 */
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <unistd.h>
#include <errno.h> 
#include <arpa/inet.h>
#include <sys/socket.h> 
#include "ewpdef.h"

#define FLAG_ACK 0x10
#define FLAG_FIN 0x01
#define FLAG_NACK 0x20



static unsigned int swapU32(unsigned int uiVal) {
	return 	((uiVal & 0xFF000000) >> 24) |
			((uiVal & 0x00FF0000) >>  8) | 
			((uiVal & 0x0000FF00) <<  8) |
			((uiVal & 0x000000FF) << 24);   
}

/* ===========================================
 * calcChecksum 
 * Simple checksum: sum all bytes, retrun sum
 * =========================================== */ 
static unsigned short calcChecksum(const unsigned char *pData, int iLen) {
	unsigned int uiSum = 0; 
	int i = 0; 
	for (i = 0; i < iLen; i += 2) {
		unsigned short usWord = (unsigned short)((unsigned short)pData[i] << 8); 
		if (i + 1 < iLen) {
			usWord |= pData[i + 1]; 
		}
		uiSum += usWord; 
		if (uiSum > 0xFFFF) {
			uiSum = (uiSum & 0xFFFF) + 1; 
		}
	} 
	return (unsigned short)(~uiSum & 0xFFFF); 
}

/* ===========================================
 * recvFull
 * Receive exactly iLen bytes
 * =========================================== */ 
static int recvFull(int iSockFd, unsigned char *pBuf, int iLen) {
	int iTotal = 0; 
	int iRet   = 0; 
	while (iTotal < iLen) {
		iRet = (int)recv(iSockFd, pBuf + iTotal, iLen - iTotal, 0); 
		if (iRet <= 0) {
			return -1; 
		}
		iTotal += iRet; 
	}
	return iTotal; 
}

/* ===========================================
 * sendAck
 * Send ACK or NACK for a given squence number 
 * =========================================== */ 
static void sendAck(int iSockFd, unsigned int uiSeqNum, int iNack) {
	struct EWA_EXAM25_TASK4_PROTOCOL_TCP stAck; 
	int iAckSize; 
	iAckSize = (int)sizeof(struct EWA_EXAM25_TASK4_PROTOCOL_TCP);
	memset(&stAck, 0, sizeof(stAck)); 
	stAck.uiAckNumber 	= swapU32(uiSeqNum); 
	stAck.uiSequenceNumber 	= 0; 
	stAck.ucFlags 		= iNack ? FLAG_NACK : FLAG_ACK; 
	stAck.usSizeOfPacket = 0; 
	stAck.usChecksum 	= calcChecksum((unsigned char *)&stAck, sizeof(stAck) - sizeof(stAck.Data)); 
	send(iSockFd, &stAck, iAckSize, 0);
	printf("Sent ACK: ack=%u flags=0x%02x bytes=%d\n", stAck.uiAckNumber, stAck.ucFlags, iAckSize); 
}

/* ===========================================
 * main
 * =========================================== */ 
int main(int argc, char *argv[]) 
{
	int 	iSockFd; 
	struct  sockaddr_in stServer; 
	char 	*pszServer; 
	int 	iPort; 
	int 	i; 
	FILE 	*pFile; 
	unsigned char *pFileBuf; 
	int 	iFileBufSize; 
	unsigned int uiExpectedSeq; 
	int 	iDone; 
	struct EWA_EXAM25_TASK4_PROTOCOL_TCP stHead; 
	unsigned char *pData; 
	int 	iHeaderSize; 
	int 	iDataSize; 
	unsigned short usCalc; 
	
	iSockFd 	= 0; 
	pszServer 	= NULL; 
	iPort 		= 0; 
	i 			= 0; 
	pFile 		= NULL; 
	pFileBuf 	= NULL; 
	iFileBufSize = 0; 
	uiExpectedSeq = 0; 
	iDone 		= 0; 
	pData 		= NULL; 
	iHeaderSize = 0; 
	iDataSize 	= 0; 
	usCalc 		= 0; 
	
	/* Parse arguments */ 
	for (i = 1; i < argc - 1; i++) {
		if (strcmp(argv[i], "-server") == 0) {
			pszServer = argv[i + 1]; 
		} else if (strcmp(argv[i], "-port") == 0) {
			iPort = atoi(argv[i + 1]); 
		}
	}
		
	if (pszServer == NULL || iPort == 0) {
		printf("Usage: %s -server <ip> -port <port>\n", argv[0]); 
		return 1; 
	}
		
	/* Create socket */ 
	iSockFd = socket(AF_INET, SOCK_STREAM, 0); 
	if (iSockFd < 0) {
		printf("Error: could not create socket\n"); 
		return 1; 
	}
		
	memset(&stServer, 0, sizeof(stServer)); 
	stServer.sin_family = AF_INET;
	stServer.sin_port   = htons((unsigned short)iPort); 
	inet_pton(AF_INET, pszServer, &stServer.sin_addr); 
	
	if (connect(iSockFd, (struct sockaddr *)&stServer, sizeof(stServer)) < 0) {
		printf("Error: could not connect to %s:%d\n", pszServer, iPort); 
		close(iSockFd); 
		return 1; 
	}
		
	printf("Connected to %s:%d\n", pszServer, iPort); 
	iHeaderSize = (int)(sizeof(struct EWA_EXAM25_TASK4_PROTOCOL_TCP) - 1); 
		
	/* Main receive loop*/ 
	while (!iDone) {
		/* Read header */ 
		memset(&stHead, 0, sizeof(stHead)); 
		if (recvFull(iSockFd, (unsigned char *)&stHead, iHeaderSize) < 0) {
			printf("Error reading header\n"); 
			break; 
		}
		
		
		printf("DEBUG before swap: sizeOfPacket raw bytes = %02x %02x\n",
				((unsigned char*)&stHead)[14], 
				((unsigned char*)&stHead)[15]);
		
		/* Read iDataSize from raw network byte order (big-endian) 
		iDataSize = (int)(unsigned short)(
				((stHead.usSizeOfPacket & 0xFF00) >> 8) |
				((stHead.usSizeOfPacket & 0x00FF) << 8));
		*/
		iDataSize = (int)stHead.usSizeOfPacket; 
		
		if (iDataSize <= 0 || iDataSize >= 65535) {
			printf("Bad size %d\n", iDataSize);
			continue;
		}
		printf("Recv: seq=%u flags=0x%02x size=%d\n",
				stHead.uiSequenceNumber, stHead.ucFlags, iDataSize);
			
		/* Read data */ 
		pData = NULL;
		if (iDataSize > 0) {
			pData = (unsigned char *)malloc(iDataSize); 
			if (pData == NULL) {
				printf("Error: malloc faild size=%d", iDataSize);
				sendAck(iSockFd, stHead.uiSequenceNumber, 1);
				continue; 
			}
				
			if (recvFull(iSockFd, pData, iDataSize) < 0) {
				printf("Error reading data\n"); 
				free(pData); 
				break; 
			}
		}
		
		/* Verify checksum */
		{	
			unsigned short usSaved; 
			unsigned short usCalc; 
			
			usSaved = stHead.usChecksum;
			stHead.usChecksum = 0;
			usCalc = calcChecksum((unsigned char *)&stHead, iHeaderSize);
			stHead.usChecksum = usSaved; 
			
			/* try to print header bytes */
			{
				int k;
				printf("Hdr(chk=0): ");  
				for (k = 0; k < iHeaderSize; k++) 
					printf("%02x ", ((unsigned char*)&stHead)[k]); 
				printf("\nsaved=0x%04x saved_swap=0x%04x calc=0x%04x\n",
						usSaved, 
						(unsigned short)(((usSaved&0xFF00)>>8)|((usSaved&0x00FF)<<8)),
						usCalc); 
			}
			
			if (usCalc != (unsigned short)(((usSaved&0xFF00)>>8) | ((usSaved&0x00FF)<<8))) {
				printf("Checksum error: saved=0x%04x calc=0x%04x\n",
						usSaved, usCalc); 
				sendAck(iSockFd, stHead.uiSequenceNumber, 1); 
				if (pData != NULL) { free(pData); pData = NULL; }
				continue; 
			}
		}
		
		/* Swap fields from network to host byte order */
		stHead.uiSequenceNumber = swapU32(stHead.uiSequenceNumber);  
		stHead.uiAckNumber = swapU32(stHead.uiAckNumber); 
		/* usSizeOfPacket already swapped above */ 
		
		
		
		/* Check sequence number */ 
		if (stHead.uiSequenceNumber != uiExpectedSeq) {
			printf("Out of sequence: got %u expected %u, sending NACK\n", 
					stHead.uiSequenceNumber, uiExpectedSeq); 
			sendAck(iSockFd, uiExpectedSeq, 1); 
			if (pData != NULL) { free(pData); pData = NULL; }
			continue; 
		}
		
		/* Store data */
		if (iDataSize > 0 && pData != NULL) {
			unsigned char *pNew; 
			pNew = (unsigned char *)malloc(iFileBufSize + iDataSize); 
			if (pNew != NULL) {
				if (pFileBuf != NULL) {
					memcpy(pNew, pFileBuf, iFileBufSize); 
					free(pFileBuf); 
				}
				memcpy(pNew + iFileBufSize, pData, iDataSize); 
				pFileBuf = pNew; 
				iFileBufSize += iDataSize; 
			}
		}
		
		/* Send ACK */ 
		sendAck(iSockFd, stHead.uiSequenceNumber, 0); 
		uiExpectedSeq = stHead.uiSequenceNumber	 + (unsigned int)iDataSize; 
		
		/* Check FIN */
		if (stHead.ucFlags & FLAG_FIN) {
			printf("FIN received, transfer complete\n"); 
			iDone = 1; 
		}
		
		if (pData != NULL) { free(pData); pData = NULL; }
	}
	
	close(iSockFd); 
	
	/* Save file */
	if (pFileBuf != NULL && iFileBufSize > 0) {
		pFile = fopen("butterfly.bmp", "wb"); 
		if (pFile != NULL) {
			fwrite(pFileBuf, 1, iFileBufSize, pFile); 
			fclose(pFile); 
			printf("File saved: butterfly.bmp (%d bytes)\n", iFileBufSize); 
		} else {
			printf("Error: could not save file\n"); 
		}
		free(pFileBuf); 
	}
	return 0; 
}



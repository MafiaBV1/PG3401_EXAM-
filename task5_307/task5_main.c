/*
 * EWA TCP server for PG3401 exam task 5
 * Isage: ./task5 -port <port> -id <servername> 
 * 
 */
 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <time.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include "ewpdef.h"

#define SERVER_IP "127.0.0.1"
#define BACKLOG 5 
#define MAX_FILENAME 256
#define MAX_FILEDATA 9999

/* =========================================================
 * buildSizeHeader
 * Fill the EWP size header with magic, size and delimiter
 * iPayloadSize is the total size of the struct being sent
 * ========================================================= */
static void buildSizeHeader(struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER *pHead, 
							int iPayloadSize) {
	char cSize[5]; 
	memcpy(pHead->acMagicNumber, "EWP", 3); 
	sprintf(cSize, "%04d", iPayloadSize); 
	memcpy(pHead->acDataSize, cSize, 4); 
	pHead->acDelimeter[0] = '|'; 							
}

/* =========================================================
 * sendServerAccept
 * Send the initial 220 accept message to the client 
 * ========================================================= */
static int sendServerAccept(int iClientFd, const char *pszServerName) {
	struct EWA_EXAM25_TASK5_PROTOCOL_SERVERACCEPT stMsg; 
	time_t tNow; 
	struct tm *pTm; 
	char cTimestamp[52]; 
	
	memset(&stMsg, 0, sizeof(stMsg)); 
	buildSizeHeader(&stMsg.stHead, (int)sizeof(stMsg)); 
	
	memcpy(stMsg.acStatusCode, "220", 3); 
	stMsg.acHardSpace[0] = ' '; 
	
	time(&tNow); 
	pTm = localtime(&tNow); 
	sprintf(cTimestamp, "%.15s SMTP %.10s %02d:%02d:%02d",
				SERVER_IP, pszServerName, pTm->tm_hour, pTm->tm_min, pTm->tm_sec); 
	memcpy(stMsg.acFormattedString, cTimestamp, 51); 
	stMsg.acHardZero[0] = '\0'; 
	return (int)send(iClientFd, &stMsg, sizeof(stMsg), 0); 
}

/* =========================================================
 * sendServerHelo
 * Send 250 HELO response with client IP and greeting 
 * ========================================================= */
static int sendServerHelo(int iClientFd, const char *pszClientIP, 
							const char *pszUsername) {
	struct EWA_EXAM25_TASK5_PROTOCOL_SERVERHELO stMsg; 
	char cGreeting[52]; 
	memset(&stMsg, 0, sizeof(stMsg));
	buildSizeHeader(&stMsg.stHead, (int)sizeof(stMsg)); 
	memcpy(stMsg.acStatusCode, "250", 3); 
	stMsg.acHardSpace[0] = ' '; 
	{
		char cUser[32]; 
		memcpy(cUser, pszUsername, 31); 
		cUser[31] = '\0'; 
		sprintf(cGreeting, "%s Hello %s", pszClientIP, cUser); 
	}
	memcpy(stMsg.acFormattedString, cGreeting, 51); 
	stMsg.acHardZero[0] = '\0'; 
	return (int)send(iClientFd, &stMsg, sizeof(stMsg), 0); 					
}

/* =========================================================
 * sendReply
 * Send a generic server reply with status code and message 
 * pszCode: "250", "354", "221", "501", etc 
 * ========================================================= */
static int sendReply(int iClientFd, const char *pszCode, const char *pszMsg) {
	struct EWA_EXAM25_TASK5_PROTOCOL_SERVERREPLY stMsg; 
	memset(&stMsg, 0, sizeof(stMsg)); 
	buildSizeHeader(&stMsg.stHead, (int)sizeof(stMsg)); 
	memcpy(stMsg.acStatusCode, pszCode, 3); 
	stMsg.acHardSpace[0] = ' '; 
	strncpy(stMsg.acFormattedString, pszMsg, 51); 
	stMsg.acFormattedString[50] = '\0'; 
	stMsg.acHardZero[0] = '\0'; 
	return (int)send(iClientFd, &stMsg, sizeof(stMsg), 0); 
}

/* =========================================================
 * recvFull
 * Receive exactly iLen bytes into pBuf
 * Returns iLen on success, -1 on error or disconnect 
 * ========================================================= */
static int recvFull(int iFd, char *pBuf, int iLen) {
	int iTotal = 0; 
	int iRet; 
	while (iTotal < iLen) {
		iRet = (int)recv(iFd, pBuf + iTotal, iLen - iTotal, 0); 
		if (iRet <= 0) {
			return -1; 
		}
		iTotal += iRet; 
	}
	return iTotal;
}

/* ========================================================
 * readSizeHeader
 * Read and parse the EWP size header from the socket 
 * Returns payload size on success, -1 on error 
 * ========================================================= */
static int readSizeHeader(int iClientFd, struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER *pHead) {
	int iHeaderSize = (int)sizeof(struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER); 
	char cSize[5]; 
	if (recvFull(iClientFd, (char *)pHead, iHeaderSize) < 0) {
		return -1; 
	}
	
	/* DEBUG 
	printf("DEBUG HDR: magic='%.3s' size='%.4s'\n", 
			pHead->acMagicNumber, pHead->acDataSize); */ 
			
	/* Verify magic */ 
	if (memcmp(pHead->acMagicNumber, "EWP", 3) != 0) {
		printf("Error: invalid magic number recived\n"); 
		return -1; 
	}
	
	memcpy(cSize, pHead->acDataSize, 4); 
	cSize[4] = '\0'; 
	return atoi(cSize); 
}


/* =========================================================
 * isValidFilename
 * Returns 1 if filename is safe (no path traversal, not empty)
 * ========================================================= */
static int isValidFilename(const char *pszName) {
	int i; 
	if (pszName == NULL || pszName[0] == '\0') return 0; 
	if (strstr(pszName, "..") != NULL) return 0; 
	if (pszName[0] == '/') return 0; 
	for (i = 0; pszName[i] != '\0'; i++) {
		char c = pszName[i]; 
		if (c == '/' || c == '\\' || c == ':') return 0; 
	}
	return 1; 
}


/* =========================================================
 * handleClient
 * Main protocol state machine for connected client 
 * ========================================================= */
static void handleClient(int iClientFd, const char *pszServerName) {
	struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER stHead; 
	struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER stFileHead; 
	int  iFilePayload; 										
	int  iFileBody; 										
	int  iPayloadSize; 
	int  iBodySize; 
	int  iHeaderSize; 
	char cBody[MAX_FILEDATA + 2]; 
	char cUsername[51]; 
	char cFilename[MAX_FILENAME]; 
	FILE *pFile; 
	int  iRunning; 
	char *pFileData; 
	
	iHeaderSize = (int)sizeof(struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER); 
	iRunning = 1; 
	
	/* Step 1: Send 220 server accept */ 
	if (sendServerAccept(iClientFd, pszServerName) < 0) {
		printf("Error sending server acceot\n"); 
		return; 
	}
	printf("Sent: 220 server accept\n"); 
	while (iRunning) {		
		/* Read size header */ 
		iPayloadSize = readSizeHeader(iClientFd, &stHead); 
		if (iPayloadSize < 0) {
			printf("Client disconnected or error reading header\n"); 
			break; 

		}
		
		/* Read remaining body */ 
		iBodySize = iPayloadSize - iHeaderSize; 
		if (iBodySize < 1 || iBodySize > MAX_FILEDATA) {
			printf("Error: invalid body size %d\n", iBodySize); 
			break; 
		}
		
		memset(cBody, 0, sizeof(cBody)); 
		if (iBodySize > 0) {
			if (recvFull(iClientFd, cBody, iBodySize) < 0) {
				printf("Error reading message body\n"); 
				break; 
			}
		}
		
		/* Identify message */ 
		if (memcmp(cBody, "HELO", 4) == 0) {
			char *pszDot; 
			printf("Recived: HELO\n"); 
			
			/* Extract username */ 
			memset(cUsername, 0, sizeof(cUsername)); 
			memcpy(cUsername, cBody + 5, 50); cUsername[50] = '\0'; 
			cUsername[50] = '\0'; 
			pszDot = strchr(cUsername, '.'); 
			if (pszDot != NULL) {
				*pszDot = '\0'; 
			}
			
			sendServerHelo(iClientFd, SERVER_IP, cUsername); 
			printf("Sent: 250 HELO response\n"); 
		}
	
		/* MAIL FROM */ 
		else if (memcmp(cBody, "MAIL FROM:", 10) == 0) {
			printf("Recived: MAIL FROM: %s\n", cBody + 11); 
			sendReply(iClientFd, "250", "Sender address ok"); 
			printf("Sent: 250 Sender address ok\n"); 
		}
		
		/* RCPT TO */ 
		else if (memcmp(cBody, "RCPT TO:", 8) == 0) {
			printf("Recived: RCPT TO: %s\n", cBody + 9); 
			sendReply(iClientFd, "250", "Recipient address ok"); 
			printf("Sent: 250 Recipient address ok\n"); 
		}
		
		/* DATA command with filename */ 
		else if (memcmp(cBody, "DATA", 4) == 0) {
			memset(cFilename, 0, sizeof(cFilename)); 
			memcpy(cFilename, cBody + 5, MAX_FILENAME - 1); cFilename[MAX_FILENAME - 1] = '\0';
			cFilename[MAX_FILENAME - 1] = '\0'; 
				
			/* Strip trailing whitespace */ 
			{
				int iLen = (int)strlen(cFilename); 
				while (iLen < 0 && (cFilename[iLen-1] == ' ' || 
									cFilename[iLen-1] == '\r' ||
									cFilename[iLen-1] == '\n')) {
					cFilename[--iLen] = '\0'; 					
				}
			}
			
			printf("Recived: DATA command, filename: %s\n", cFilename); 
				
			if (!isValidFilename(cFilename)) {
				sendReply(iClientFd, "501", "invalid fielname"); 
				printf("Sent: 501 Invalid filename\n"); 
				continue; 
			}
			sendReply(iClientFd, "354", "ready for message"); 
			printf("Sent: 354 Ready for message\n"); 
			
			/* Now revece the file data pacet */
			iFilePayload = readSizeHeader(iClientFd, &stFileHead); 
			if (iPayloadSize < 0) {
				printf("Errpr reading file data header\n"); 
				break; 
			}
			iFileBody = iFilePayload; 
			/*
			 * printf("DEBUG FILE: iFilePaylod=%d iFileBody=%d\n", iFilePayload, iFileBody); 
			 * printf("DEBUG: iPayloadSize=%d iHeaderSize=%d iBodySize=%d\n", iPayloadSize, iHeaderSize, iBodySize); 
			*/
			
			/* Bytte iBodySize med iFileBody (281-311) */  
			if (iFileBody > MAX_FILEDATA) { 
				printf("Error: invalid file data size %d\n", iFileBody); 
				break; 
			}
			
			pFileData = (char *)malloc(iFileBody + 1); 
			if (pFileData == NULL) {
				printf("Error: malloc failed for file data\n"); 
				break;  
			}
			memset(pFileData, 0, iFileBody + 1); 
			if (recvFull(iClientFd, pFileData, iFileBody) < 0) {
				printf("Error reading file data\n"); 
				free(pFileData); 
				break; 
			}
			
			/* Drain trailing new line if present */ 
			{
				char cDrain[1]; 
				recv(iClientFd, cDrain, 1, MSG_DONTWAIT); 
			}
			
			/* Write to file */ 
			pFile = fopen(cFilename, "wb"); 
			if (pFile == NULL) {
				printf("Error: could not create file %s\n", cFilename); 
				sendReply(iClientFd, "501", "Could not create file"); 
				free(pFileData); 
				continue; 	
			}
			
			fwrite(pFile, 1, iFileBody, pFile); 
			fclose(pFile); 
			free(pFileData); 
			
			printf("File %s saved (%d bytes)\n", cFilename, iFileBody); 
			sendReply(iClientFd, "250", "File received ok"); 
			printf("Sent: 250 File received ok\n"); 
		}
		
		/* QUIT */ 
		else if (memcmp(cBody, "QUIT", 4) == 0) {
			printf("Received: QUIT\n"); 
			sendReply(iClientFd, "221", "Goodbye"); 
			printf("Sent: 221 Goodbye\n"); 
			iRunning = 0; 
			break; 
		}
		
		else {
			printf("Received unkown command, ignoring\n"); 
		}
	}
}

/* =========================================================
 * parsArgs
 * Pars -port and -id from argv
 * Returns 1 on success, 0 on failure
 * ========================================================= */
static int parseArgs(int argc, char *argv[], int *piPort, char *pszID, int iIDLen) {
	int i; 
	*piPort = 0; 
	pszID[0] = '\0'; 
	
	for (i = 1; i < argc - 1; i++) {
		if (strcmp(argv[i], "-port") == 0) {
			*piPort = atoi(argv[i + 1]); 
		} else if (strcmp(argv[i], "-id") == 0) {
			strncpy(pszID, argv[i + 1], iIDLen - 1); 
			pszID[iIDLen - 1] = '\0'; 
		}
	}
	
	if (*piPort == 0 || pszID[0] == '\0') {
		printf("Usage: %s -port <port> -id <servername>\n", argv[0]); 
		return 0; 
	}
	return 1; 
}

/* =========================================================
 * main
 * ========================================================= */
int main(int argc, char *argv[]) {
	int 				iPort; 
	char 				cID[64]; 
	int 				iServerFd; 
	int 				iClientFd; 
	struct sockaddr_in stServerAddr; 
	struct sockaddr_in  stClientAddr; 
	socklen_t 			iClientLen; 
	int 				iOpt; 
	
	if (!parseArgs(argc, argv, &iPort, cID, (int)sizeof(cID))) {
		return 1; 
	}
	printf("Starting EWP server: id=%s port=%d\n", cID, iPort); 
	
	/* Create TCP socket */ 
	iServerFd = socket(AF_INET, SOCK_STREAM, 0); 
	if (iServerFd < 0) {
		perror("socket"); 
		return 1; 
	}
	
	/* Allow resue of address */ 
	iOpt = 1; 
	setsockopt(iServerFd, SOL_SOCKET, SO_REUSEADDR, &iOpt, sizeof(iOpt)); 
	
	/* Bind to loopback */ 
	memset(&stServerAddr, 0, sizeof(stServerAddr)); 
	stServerAddr.sin_family = AF_INET; 
	stServerAddr.sin_port = htons((unsigned short)iPort); 
	stServerAddr.sin_addr.s_addr = inet_addr(SERVER_IP); 
	if (bind(iServerFd, (struct sockaddr *)&stServerAddr, sizeof(stServerAddr)) < 0) {
		perror("bind"); 
		close(iServerFd); 
		return 1; 
	}
	
	if (listen(iServerFd, BACKLOG) < 0) {
		 perror("listen"); 
		 close(iServerFd); 
		 return 1; 
	}
	printf("Listening on %s:%d\n", SERVER_IP, iPort); 
	printf("Waiting for EWA connection...\n"); 
	
	/* Accept one client at a time in a loop */ 
	while (1) {
		iClientLen = sizeof(stClientAddr); 
		iClientFd = accept(iServerFd, (struct sockaddr *)&stClientAddr, &iClientLen); 
		if (iClientFd < 0) {
			perror("accept"); 
			continue; 
		}
		printf("Client connected: %s\n", inet_ntoa(stClientAddr.sin_addr));
		handleClient(iClientFd, cID); 
		close(iClientFd); 
		printf("Client disconnected\n"); 
	}
	close(iServerFd); 
	return 0;
}


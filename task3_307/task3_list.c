/*
 * Implementation of al doubly linked list operations 
 * for the exam grading system 
 */
 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include "task3.h"  

/* =========================================================
 * initList 
 * Initialise an ExamList to empty state 
 * iNumTasks, how many tasks this exam has 
 * piMaxPoints, array of max points for each task
 * ========================================================= */
void initList(ExamList *pList, int iNumTasks, int *piMaxPoints) {
	int i; 
	pList->pHead = NULL; 
	pList->pTail = NULL; 
	pList->iCount = 0; 
	pList->iNumTasks = iNumTasks; 
	for (i = 0; i < iNumTasks; i++) {
		pList->aiMaxPoints[i] = piMaxPoints[i]; 
	}
} 

/* ==========================================================
 * freeList
 * Walk the list and free every node, then rest the list struct 
 * =========================================================== */
void freeList(ExamList *pList) {
	Candidate *pCur = pList->pHead; 
	Candidate *pNext = NULL; 
	while (pCur != NULL) {
		pNext = pCur->pNext; 
		free(pCur); 
		pCur = pNext; 
	}	
	
	pList->pHead = NULL; 
	pList->pTail = NULL; 
	pList->iCount = 0; 
} 

/* =========================================================
 * findCandidate (internal helper) 
 * Returns pointer to the candidate with matching ID, or NULL 
 * ========================================================== */ 
static Candidate *findCandidate(const ExamList *pList, const char *pszID) {
	Candidate *pCur = pList->pHead; 
	
	while (pCur != NULL) {
		if (strncmp(pCur->szCandidateID, pszID, MAX_ID_LEN) == 0) {
			return pCur; 
		}
		pCur = pCur->pNext; 
	}
	return NULL; 
}

/* =========================================================
 * updateScore 
 * Recalculates iScore for a candidate from all graded tasks 
 * ========================================================= */ 
static void updateScore(Candidate *pCand) {
	int i; 
	pCand->iScore = 0; 
	for (i = 0; i < pCand->iNumTasks; i++) {
		if (pCand->atTasks[i].iGraded) {
			pCand->iScore += pCand->atTasks[i].iPoints; 
		}
	}
}

/* =========================================================
 * addCandidate
 * Allocate a new Candidate node and append it to the list tail
 * Returns 1 on success, 0 on failure (malloc or duplicateID) 
 * ========================================================= */ 
int addCandidate(ExamList *pList, const char *pszID) {
	Candidate *pNew = NULL; 
	int i; 
	int iLen; 
	int iMax; 
	
	/* Reject duplicate IDs */ 
	if (findCandidate(pList, pszID) != NULL) {
		printf("Error: candidate %s already exists\n", pszID); 
		return 0; 
	}
	
	pNew = (Candidate *)malloc(sizeof(Candidate)); 
	if (pNew == NULL) {
		printf("Error: malloc faild for new candidate\n"); 
		return 0; 
	}
	
	/* Initialise the new node 
	strncpy(pNew->szCandidateID, pszID, MAX_ID_LEN - 1); 
	pNew->szCandidateID[MAX_ID_LEN - 1] = '\0'; 
	*/ 
	/* switch out strncpy */
	iMax = MAX_ID_LEN - 1; 
	for (iLen = 0; iLen < iMax && pszID[iLen] != '\0'; iLen++) {
		pNew->szCandidateID[iLen] = pszID[iLen];
	}
	pNew->szCandidateID[iLen] = '\0'; 
	
	pNew->iNumTasks = pList->iNumTasks; 
	pNew->iScore = 0; 
	pNew->pPrev = NULL; 
	pNew->pNext = NULL; 
	
	for (i = 0; i < pList->iNumTasks; i++) {
		pNew->atTasks[i].iPoints = 0; 
		pNew->atTasks[i].iMaxPoints = pList->aiMaxPoints[i]; 
		pNew->atTasks[i].iGraded = 0; 
		pNew->atTasks[i].szJustification[0] = '\0'; 
	}
	
	
	/* Append to tail */ 
	if (pList->pTail == NULL) {
		/* pList is empty */ 
		pList->pHead = pNew; 
		pList->pTail = pNew; 
	} else {
		pNew->pPrev = pList->pTail; 
		pList->pTail->pNext = pNew; 
		pList->pTail = pNew; 
	}
	
	pList->iCount++; 
	printf("Candidate %s added\n", pszID); 
	return 1; 
}

/* =========================================================
 * loadFromFile 
 * Read candidate IDs from a plain text file (one ID per line) 
 * if line start's with '#' are skipped 
 * Returns number of candidates successfully added 
 * ========================================================= */ 
int loadFromFile(ExamList *pList, const char *pszFilename) {
	FILE *pFile = NULL; 
	char szLine[256]; 
	int iAdded = 0; 
	int iLen = 0; 
	
	pFile = fopen(pszFilename, "r"); 
	if (pFile == NULL) {
		printf("Error: could not open file %s\n", pszFilename); 
		return 0; 
	}
	
	while (fgets(szLine, sizeof(szLine), pFile) != NULL) {
		/* Strip trailing newline */ 
		iLen = (int)strlen(szLine); 
		if (iLen > 0 && szLine[iLen - 1] == '\n') {
			szLine[iLen - 1] = '\0'; 
			iLen--; 
		}
		
		/* Skip empty lines and comment lines (lines starting with #) */ 
		if (iLen == 0 || szLine[0] == '#' || szLine[0] == '\r') {
			continue; 
		}
		
		if (addCandidate(pList, szLine)) {
			iAdded++; 
		}
	}
	
	fclose(pFile); 
	printf("Loaded %d condidates from %s\n", iAdded, pszFilename); 
	return iAdded; 
}


/* =========================================================
 * gradeTask 
 * Set points and justification for one task of one candidate
 * iTaskNum is 1-based (task 1 = index 0) 
 * Returns 1 on success, 0 on failure 
 * ========================================================= */ 
int gradeTask(ExamList *pList, const char *pszID, 
				int iTaskNum, int iPoints, const char *pszJustification) {
	Candidate *pCand = NULL; 
	int iIndex = iTaskNum - 1; 
	pCand = findCandidate(pList, pszID); 
	if (pCand == NULL) {
		printf("Error: candidate %s not found\n", pszID); 
		return 0; 
	}
	
	if (iIndex < 0 || iIndex >= pCand->iNumTasks) {
		printf("Error: tasknumber %d is out of range (1-%d)\n", 
				iTaskNum, pCand->iNumTasks); 
		return 0; 
	}	
	
	if (iPoints < 0 || iPoints > pCand->atTasks[iIndex].iMaxPoints) {
		printf("Error: points %d out of range (0-%d)\n", 
				iPoints, pCand->atTasks[iIndex].iMaxPoints); 
		return 0; 
	}
	
	pCand->atTasks[iIndex].iPoints = iPoints; 
	pCand->atTasks[iIndex].iGraded = 1; 
	strncpy(pCand->atTasks[iIndex].szJustification, 
			pszJustification, MAX_JUST_LEN - 19); 
	pCand->atTasks[iIndex].szJustification[MAX_JUST_LEN - 1] = '\0';
	updateScore(pCand); 
	printf("Task%d graded for candidate %s: %d pts\n", 
			iTaskNum, pszID, iPoints); 
	return 1;
}

/* =========================================================
 * calculateMax
 * Returns the sum of all max points for this exam 
 * ========================================================= */ 
int calcTotalMax(const ExamList * pList) {
	int i; 
	int iTotal = 0; 
	for (i = 0; i < pList->iNumTasks; i++) {
		iTotal += pList->aiMaxPoints[i]; 
	}
	return iTotal; 
}

/* =========================================================
 * calcGrade 
 * Returns letter grade A-F based on score percentage 
 * ========================================================= */ 
char calcGrade(int iScore, int iTotalMax) {
	int iPercent; 
	if (iTotalMax <= 0) {
		return 'F'; 
	}
	iPercent = (iScore * 100) / iTotalMax; 
	if (iPercent >= GRADE_A) return 'A';
	if (iPercent >= GRADE_B) return 'B';
	if (iPercent >= GRADE_C) return 'C';
	if (iPercent >= GRADE_D) return 'D';
	if (iPercent >= GRADE_E) return 'E'; 
	return 'F'; 
}

/* =========================================================
 * isFullyGraded
 * Returns 1 if all tasks for a candidate ave been graded 
 * ========================================================= */
static int isFullyGraded(const Candidate *pCand) {
	int i; 
	for (i = 0; i < pCand->iNumTasks; i++) {
		if (!pCand->atTasks[i].iGraded) {
			return 0; 
		}
	}
	return 1; 
}

/* =========================================================
 * printAllWithGrades
 * Print every candidate with their total score and grade 
 * ========================================================= */
void printAllWithGrades(const ExamList *pList) {
	Candidate *pCur = pList->pHead; 
	int iTotalMax = calcTotalMax(pList); 
	char cGrade; 
	printf("\n--- All Candidates ---\n"); 
	printf("%-15s %8s %8s %6s\n", "CandidateID", "Score", "MaxPts", "Grade"); 
	printf("%-15s %8s %8s %6s\n", "-------------", "-----", "------", "-----"); 
	while (pCur != NULL) {
		cGrade = calcGrade(pCur->iScore, iTotalMax); 
		printf("%-15s %8d %8d %6c\n", 
				pCur->szCandidateID,
				(int)pCur->iScore, 
				(int)iTotalMax,
				(int)cGrade); 
		pCur = pCur->pNext;  
	}
	printf("--------------------------\n"); 
	printf("Total candidates: %d\n\n", pList->iCount); 
}

/* =========================================================== 
 * printByGrade
 * Print only candidates whose calculated grade matches cGrade 
 * =========================================================== */
void printByGrade(const ExamList *pList, char cGrade) {
	Candidate *pCur = pList->pHead; 
	int iTotMax = calcTotalMax(pList); 
	int iFound = 0; 
	char cCalc; 
	
	printf("\n--- Candidates with grade %c ---\n", cGrade); 
	while (pCur != NULL) {
		cCalc = calcGrade(pCur->iScore, iTotMax); 
		if (cCalc == cGrade) {
			printf("  %-15s Score: %d/%d\n", 
					pCur->szCandidateID, 
					pCur->iScore, 
					iTotMax); 
			iFound++;
		}
		pCur = pCur->pNext; 
	}
	
	if (iFound == 0) {
		printf("   (none)\n"); 
	}
	printf("--- %d candidate(s) found ---\n\n", iFound); 
}

/* =========================================================
 * printUngraded 
 * Print all candidates where at least one task is not graded 
 * ========================================================= */
void printUngraded(const ExamList * pList) {
	Candidate *pCur = pList->pHead; 
	int iFound = 0; 
	int i; 
	
	printf("\n---- Ungraded / Incomplete Candidates ---\n"); 
	while (pCur != NULL) {
		if (!isFullyGraded(pCur)) {
			printf(" %-15s Missing tasks: ", pCur->szCandidateID); 
			for (i = 0; i < pCur->iNumTasks; i++) {
				if (!pCur->atTasks[i].iGraded) {
					printf("%d ", i + 1); 
				}
			}
			printf("\n"); 
			iFound++; 
		}
		pCur = pCur->pNext; 
	}
	
	if (iFound == 0) {
		printf(" All candidates are fully graded\n"); 
	}
	printf("--- %d candidate(s) not fully graded ---\n\n", iFound); 
}

/* =========================================================
 * printCandidate
 * Print detailed grade report for one specific candidate 
 * ========================================================= */
void printCandidate(const ExamList *pList, const char *pszID) {
	Candidate *pCand = findCandidate(pList, pszID); 
	int iTotalMax = calcTotalMax(pList); 
	int i; 
	char cGrade; 
	
	if (pCand == NULL) {
		printf("Error: candidate %s not found\n", pszID); 
		return; 
	}
	
	cGrade = calcGrade(pCand->iScore, iTotalMax); 
	printf("\n--- Candidate: %s ---\n", pCand->szCandidateID); 
	printf("Final score: %d / %d | Grade: %c\n",
			pCand->iScore, iTotalMax, cGrade); 
	printf("---\n");
	for (i = 0; i < pCand->iNumTasks; i++) {
		printf("Task %d: ", i + 1); 
		if (pCand->atTasks[i].iGraded) {
			printf("%d / %d pts\n",
				pCand->atTasks[i].iPoints, 
				pCand->atTasks[i].iMaxPoints); 
			printf("	Justification: %s\n", 
				pCand->atTasks[i].szJustification);
		} else {
			printf("(not graded yet) / %d pts\n", 
					pCand->atTasks[i].iMaxPoints); 
		}
	} 
	printf("---\n\n"); 
}



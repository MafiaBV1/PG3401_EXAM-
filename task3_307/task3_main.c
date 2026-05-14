/*
 * Entry point and interactive menu for the exam grading system
 * All list state lives in a local ExamList variable 
 */
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include "task3.h" 

/* =========================================================
 * printMenu
 * Disply the main menu options to the user 
 * ========================================================= */ 
static void printMenu(void) {
	printf("\n---- Exam Grading System ----\n"); 
	printf("1. Add candidate manually\n"); 
	printf("2. Load candidates from file\n"); 
	printf("3. Grade a task for a candidate\n"); 
	printf("4. Print all candidates with grades\n"); 
	printf("5. Print candidates by grade\n");
	printf("6. print ungraded / incomplete candidates\n"); 
	printf("7. Print full report for one candidate\n"); 
	printf("0. Exit\n"); 
	printf("----------------------------------\n"); 
	printf("Choose an option: "); 
}

/* =========================================================
 * menuAddManual
 * Prompt user for a candidate ID and add it 
 * ========================================================= */
static void menuAddManual(ExamList *pList) {
	char szID[MAX_ID_LEN]; 
	printf("Enter candidate ID: "); 
	if (fgets(szID, sizeof(szID), stdin) == NULL) {
	
		return; 
	}
	/* Strip newLine */
	szID[strcspn(szID, "\n")] = '\0'; 
	addCandidate(pList, szID); 
}

/* =========================================================
 * menuLoadFile 
 * Prompt user for a filename and load candidates from it 
 * ========================================================= */
static void menuLoadFile(ExamList *pList) {
	char szFilename[128]; 
	printf("Enter filname: "); 
	if (fgets(szFilename, sizeof(szFilename), stdin) == NULL) {
		return; 
	}
	szFilename[strcspn(szFilename, "\n")] = '\0'; 
	loadFromFile(pList, szFilename); 
}

/* =========================================================
 * menuGradeTask
 * Prompt for candidate ID, task number, points, justification 
 * ========================================================= */
static void menuGradeTask(ExamList *pList) {
	char szID[MAX_ID_LEN]; 
	char szJust[MAX_JUST_LEN]; 
	char szBuf[32]; 
	int iTask; 
	int iPoints = 0; 
	
	printf("Enter candidate ID: "); 
	if (fgets(szID, sizeof(szID), stdin) == NULL) return; 
	szID[strcspn(szID, "\n")] = '\0'; 
	
	printf("Task number (1-%d): ", (int)pList->iNumTasks); 
	if (fgets(szBuf, sizeof(szBuf), stdin) == NULL) return; 
	if (sscanf(szBuf, "%d", &iTask) != 1) return; 
	
	printf("Points awarded; "); 
	if (fgets(szBuf, sizeof(szBuf), stdin) == NULL) return; 
	if (sscanf(szBuf, "%d", &iPoints) != 1) return; 
	
	printf("Justification: "); 
	if (fgets(szJust, sizeof(szJust), stdin) == NULL) return;
	szJust[strcspn(szJust, "\n")] = '\0'; 
	gradeTask(pList, szID, iTask, iPoints, szJust); 
}

/* =========================================================
 * menuPrintByGrade
 * Ask user for a grade letter, then print matching candidates
 * ========================================================= */
static void menuPrintByGrade(const ExamList *pList) {
	char szInput[8]; 
	char cGrade; 
	printf("Enter grade letter (A/B/C/D/E/F): "); 
	if (fgets(szInput, sizeof(szInput), stdin) == NULL) return; 
	
	cGrade = szInput[0]; 
	if (cGrade >= 'a' && cGrade <= 'f') {
		cGrade = (char)(cGrade - 32); /* Conert to uppercase */ 
	}
	
	if (cGrade < 'A' || cGrade > 'F') {
		printf("Invalid grade '%c'\n", cGrade); 
		return; 
	}
	printByGrade(pList, cGrade); 
}

/* =========================================================
 * menuPrintOne
 * Ask user for a candidate ID and print their full report 
 * ========================================================= */
static void menuPrintOne(const ExamList *pList) {
	char szID[MAX_ID_LEN]; 
	printf("Enter candidate ID: "); 
	if (fgets(szID, sizeof(szID), stdin) == NULL) return; 
	szID[strcspn(szID, "\n")] = '\0'; 
	printCandidate(pList, szID); 
}

/* =========================================================
 * setupExam
 * Ask the user how many tasks this exam has and thier max pts
 * Returns 1 on success, 0 on invalid input 
 * ========================================================= */
static int setupExam(ExamList *pList) {
	int iNumTasks; 
	int aiMaxPoints[MAX_TASKS]; 
	int i; 
	char szBuf[32]; 
	
	printf("--- Exam Setup ---\n"); 
	printf("Number of tasks (1-%d): ", MAX_TASKS); 
	if (fgets(szBuf, sizeof(szBuf), stdin) == NULL) return 0; 
	if (sscanf(szBuf, "%d", &iNumTasks) != 1 ||
		iNumTasks < 1 || iNumTasks > MAX_TASKS) {
		printf("Invalid number of tasks\n"); 
		return 0; 	
	}
	
	for (i = 0; i < iNumTasks; i++) {
		printf("Max points for task %d: ", i + 1); 
		if (fgets(szBuf, sizeof(szBuf), stdin) == NULL) return 0;
		if (sscanf(szBuf, "%d", &aiMaxPoints[i]) != 1 || aiMaxPoints[i] < 1) {
			printf("Invalid max points\n"); 
			return 0; 
		}
	}
	
	initList(pList, iNumTasks, aiMaxPoints); 
	printf("Exam configured: %d task(s)\n", iNumTasks); 
	return 1; 
}

/* =========================================================
 * main
 * Sets up the exam, then runs the interactive menu loop
 * ========================================================= */
int main(void) {
	ExamList tList; /* All state live here */ 
	int iChoice; 
	int iRunning = 1; 
	
	if (!setupExam(&tList)) {
		printf("Setup faild. Exiting\n"); 
		return 1; 
	}
	
	while (iRunning) {
		char szBuf[16]; 
		printMenu(); 
		if (fgets(szBuf, sizeof(szBuf), stdin) == NULL) {
			break; 
		}
		if (sscanf(szBuf, "%d", &iChoice) != 1) {
			continue; 
		}
		
		switch (iChoice) {
			case 1: 
				menuAddManual(&tList); 
				break;
			case 2: 
				menuLoadFile(&tList); 
				break;
			case 3: 
				menuGradeTask(&tList); 
				break;
			case 4: 
				printAllWithGrades(&tList); 
				break;
			case 5: 
				menuPrintByGrade(&tList); 
				break;
			case 6: 
				printUngraded(&tList); 
				break;
			case 7: 
				menuPrintOne(&tList); 
				break;
			case 0: 
				iRunning = 0;  
				break; 
			default: 
			printf("Unknown option. Try again\n"); 
			break; 
			
		}
	}
	
	/* Always clean up before exit */
	freeList(&tList); 
	printf("Goodbye\n"); 
	return 0; 
}
 


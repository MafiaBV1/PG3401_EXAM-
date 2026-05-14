/*
 * Header file for Task 3: Exam submission grading system
 * Double linked list of exam candidates
 */

#ifndef TASK3_H
#define TASK3_H

/* Maximum number of tasks in an exam */ 
#define MAX_TASKS 10 

/* Maximum string lenghts */ 
#define MAX_ID_LEN 20 
#define MAX_JUST_LEN 256

/* Grade thershold (percentage of total possible points) */ 
#define GRADE_A 90
#define GRADE_B 80
#define GRADE_C 65
#define GRADE_D 50
#define GRADE_E 40

/* --------------------------------------------
 * Struct: TaskEntry
 * Holds points and justification for one task
 * -------------------------------------------- */
typedef struct TaskEntry {
	int iPoints; 
	int iMaxPoints; 
	int iGraded; 
	char szJustification[MAX_JUST_LEN]; 
} TaskEntry; 

/* --------------------------------------------
 * Struct: Candidate
 * One node in the doubly linked list 
 * -------------------------------------------- */
typedef struct Candidate {
	char szCandidateID[MAX_ID_LEN]; 
	TaskEntry atTasks[MAX_TASKS]; 
	int iNumTasks; 
	int iScore; 
	struct Candidate *pPrev; 
	struct Candidate *pNext; 
} Candidate; 

/* ----------------------------------------------------
 * Struct: ExamList
 * Wrapper for the doubly linked list + exam metadata
 * ---------------------------------------------------- */
typedef struct ExamList {
	Candidate *pHead; 
	Candidate *pTail; 
	int iCount; 
	int iNumTasks; 
	int aiMaxPoints[MAX_TASKS]; 
} ExamList; 

/* --------------------------------------------
 * List management 
 * -------------------------------------------- */
void initList (ExamList *pList, int iNumTasks, int *piMaxPoints); 
void freeList (ExamList *pList); 

/* --------------------------------------------
 * Candidate opertaions 
 * -------------------------------------------- */
int addCandidate	(ExamList *pList, const char *pszID); 
int loadFromFile 	(ExamList *pList, const char *pszFliename); 
int gradeTask		(ExamList *pList, const char *pszID,
					int iTaskNum, int iPoints, 
					const char *pszJustification); 
 
/* --------------------------------------------
 * Reporting 
 * -------------------------------------------- */
void printAllWithGrades (const ExamList *pList); 
void printByGrade 		(const ExamList *pList, char cGrade); 
void printUngraded 		(const ExamList *pList); 
void printCandidate 	(const ExamList *pList, const char *pszID); 

/* --------------------------------------------
 * Helpers 
 * -------------------------------------------- */
char calcGrade (int iScore, int iTotalMax); 
int calcTotalMax (const ExamList *pList); 

#endif /* TASK3_H */ 
 


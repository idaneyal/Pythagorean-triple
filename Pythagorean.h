#ifndef PYTHAGOREAN_H
#define PYTHAGOREAN_H



#include "Common.h"
#include "Threads.h"


typedef struct _PythagoreanTriple
{
	int n;
	int m;
	int a;
	int b;
	int c;
	struct _PythagoreanTriple *next;
}PythagoreanTriple;

int CheckIfCoprimeNumbers(int m, int n);
void CalculatePythagoreanNumbers(int m, int n, int *FirstNumber, int *SecondNumber, int *ThridNumber);
PythagoreanTriple* CreatePythagorean(int m, int n);
PythagoreanTriple* AddMemberToList(PythagoreanTriple *ListHead, PythagoreanTriple *Member);
PythagoreanTriple* FindMemberLocation(int m, int n, PythagoreanTriple *ListHead);
BOOL CheckIfCanAccessNumber(int Number);
void FindTriples(int Number);
void FindAllTriples(ParamStruct *param);
void InsertToBuffer(PythagoreanTriple *member);
void OpenInsertBufferGuards();
void CloseInsertBufferGuards();
BOOL CheckIfBreak(ParamStruct *param);
BOOL OpenExtractBufferGuards();
void CloseExtractBufferGuards();
void SortList(ParamStruct *param);
void UpdateStaticParameters(HANDLE **MArr, int **VArr, int maxnumber, PythagoreanTriple **buffer, PythagoreanTriple **listhead);
void PrintList(char FileName[]);
void OpenMutexAndSemaphore(LPCSTR BufferMutexName, int BufferSize);
void FreeList(PythagoreanTriple *List);


#endif // !PYTHAGOREAN_H


#pragma once

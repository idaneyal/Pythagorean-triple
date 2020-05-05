
#ifndef THREADS_H
#define THREADS_H


#include "Common.h"

typedef struct _ParamStruct
{
	BOOL *ThreadsFinish;
}ParamStruct;

extern void FindAllTriples(ParamStruct *param);
extern void SortList(ParamStruct *param7);
int CreateMutexArr(int Size, HANDLE **MutexArr);
int CreateSemaphoreSimple(int Size, LPCSTR EmptyCells, LPCSTR FullCells, HANDLE *Full, HANDLE *Empty);
HANDLE CreateThreadSimple(LPTHREAD_START_ROUTINE p_start_routine, LPDWORD *p_thread_id, LPVOID *param);
DWORD WINAPI FindPythagoreanTriple(LPVOID lpParam);
DWORD WINAPI SortListThread(LPVOID lpParam);
int CloseThreads(HANDLE CalculationThreads[], int size, HANDLE SortThread);
void CloseAllHandles(HANDLE MutexArr[], int MaxNumber, LPCSTR EmptySemaphoreName, LPCSTR FullSemaphoreName, LPCSTR BufferMutexName,
	HANDLE ThreadArr[], int ThreadNumber);
void CreateSortThread(ParamStruct *param, LPDWORD *p_thread_id, HANDLE *SortThread);

#endif // !THREADS_H


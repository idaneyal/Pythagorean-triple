/*
Description:	This modul is in charge of the opening of the threads, mutex and semaphores and their handling.
*/
#include "Threads.h"



/*
Description:	This function create a thread

Parameters:		LPTHREAD_START_ROUTINE p_start_routine - a function which will be open by the thread
				LPDWORD* p_thread_id - a pointer to the thread id
				LPVOID *param - a pointer to the parameter which the thread will pass to the function it open

Retrun:			this function return the handle of the thread
*/

HANDLE CreateThreadSimple(LPTHREAD_START_ROUTINE p_start_routine, LPDWORD *p_thread_id, LPVOID *param)
{
	HANDLE thread_handle;

	if (NULL == p_start_routine)
	{
		printf("Error when creating a thread");
		printf("Received null pointer");
		exit(-1);
	}
	thread_handle = CreateThread(NULL, 0, p_start_routine, param, 0, p_thread_id);
	if (thread_handle == NULL)
	{
		printf("An error occured during the creation of the thread\n"
			"Exiting program...\n");
		exit(-1);
	}
	return thread_handle;

}

/*
Description:	This function is called by each of the calculation threads
*/

DWORD WINAPI FindPythagoreanTriple(LPVOID lpParam)
{
	ParamStruct *param = (ParamStruct*)lpParam;
	FindAllTriples(param);
	return 0;
}

/*
Description:	This function is called by the sort thread
*/

DWORD WINAPI SortListThread(LPVOID lpParam)
{
	ParamStruct *param = (ParamStruct*)lpParam;
	SortList(param);
	return 0;
}

/*
Description:	This function close the calculation and sort threads and print if an error occured

Arguments:		HANDLE CalculationThreads[]	- the calculation threads handle array
				int size					- the size of the calculation threads array
				HANDLE SortThread			- the sort thread handle

Algorithm:		for loop
					check for each calculation thread if the wait code or exit code are not what expected
						if it is print a proper message and return FAIL
				check if the wait code and exit code of the sort thread are as expected

RETURN:			This function return FAIL if one of the threads had any error during it run
				This function return SUCCESS if all the threads ended without any proble,
*/

int CloseThreads(HANDLE CalculationThreads[], int size, HANDLE SortThread)
{
	int i, result = 0;
	DWORD waitcode, exitcode=0;

	for (i = 0; i < size; i++)
	{
		waitcode = WaitForSingleObject(CalculationThreads[i], INFINITE);
		if (waitcode != WAIT_OBJECT_0)
		{
			printf("Calculation Thread number %d, has ended with waitcode %d.\n Exiting program", i, waitcode);
			return FAIL;
		}
		GetExitCodeThread(CalculationThreads[i], &exitcode);
		if (exitcode != 0)
		{
			printf("Calculation Thread number %d, has crashed with exitcode %d.\n Exiting program",i,exitcode);
			return FAIL;
		}		
	}
	waitcode = WaitForSingleObject(SortThread, INFINITE);
	if (waitcode != WAIT_OBJECT_0)
	{
		printf("Sort Thread has ended with waitcode %d.\n Exiting program", waitcode);
		return FAIL;
	}
	GetExitCodeThread(SortThread, &exitcode);
	if (exitcode != 0)
	{
		printf("Sort Thread has crashed with exitcode %d.\n Exiting program", exitcode);
		return FAIL;
	}
	return SUCCESS;
}

/*
Description:	This function create an array of mutex with size of MAX_NUMBER argument

Arguments:		HANDLE **MutextArr	- a pointer to an array of mutex with size of MAX_NUMBER
				int Size			- the size of the mutex array

Returns:		SUCCESS	- the value of 0 if the function ended without errors
				FAIL	- an error occured during the function
*/

int CreateMutexArr(int Size, HANDLE **MutexArr)
{
	int i;
	*MutexArr = (HANDLE*)malloc(Size * sizeof(HANDLE));
	if (*MutexArr == NULL) {
		printf("An Error occured during the creation of the mutex array\n"
			"Exiting program...\n");
		return FAIL;
	}
	for (i = 0; i < Size; i++) {
		(*MutexArr)[i] = CreateMutex(NULL, FALSE, NULL);
		if ((*MutexArr)[i] == NULL) {
			printf("An error occured during Createmutex number %d\n"
				"Exiting program....\n", i);
			return FAIL;
		}
	}
	return SUCCESS;
}

/*
Description:	This function open the semaphores of the buffer

Arguments:		int Size			- the size of the buffer
				LPCSTR EmptyCells	- the name of the EmptyCells semaphore
				LPCSTR FullCells	- the name of the FullCells semaphore
				HANDLE *Full		- a pointer to the full semaphore
				HANDLE *Empty		- a pointer to the Empty semaphore

Returns:		The function return FAIL if an error occured during the Creation of the semaphors
				The function return SUCCESS if it function worked without problems
*/

int CreateSemaphoreSimple(int Size, LPCSTR EmptyCells, LPCSTR FullCells, HANDLE *Full, HANDLE *Empty)
{
	*Empty = CreateSemaphore(NULL, Size, Size, EmptyCells);
	*Full = CreateSemaphore(NULL, 0, Size, FullCells);
	if (*Empty == NULL || *Full == NULL)
	{
		printf("An Error occured during the creation of the buffer semaphores and mutex\n"
			"Exiting program...\n");
		return FAIL;
	}
	return SUCCESS;
}

/*
Description:	This function close all the mutex and semaphores of the program

Arguments:		HANDLE MutexArr[]			- a Handle array of the mutex of the numbers
				int Size					- the size of the mutex array
				LPCSTR EmptySemaphoreName	- the name of the empty semaphore
				LPCSTR FullSemaphoreName	- the name of the full semaphore
				LPCSTR BufferMutexName		- the name of the mutex of the buffer
*/

void CloseAllHandles(HANDLE MutexArr[], int MaxNumber, LPCSTR EmptySemaphoreName, LPCSTR FullSemaphoreName, LPCSTR BufferMutexName,
	HANDLE ThreadArr[], int ThreadNumber)
{
	HANDLE BufferMutex,TempSemahpore;
	int i;
	for (i = 0; i < MaxNumber; i++)
	{
		CloseHandle(MutexArr[i]);
	}
	BufferMutex = OpenMutex(SYNCHRONIZE, FALSE, BufferMutexName);
	TempSemahpore = OpenSemaphore(SYNCHRONIZE, FALSE, FullSemaphoreName);
	CloseHandle(BufferMutex);
	CloseHandle(TempSemahpore);
	TempSemahpore = OpenSemaphore(SYNCHRONIZE, FALSE, EmptySemaphoreName);
	CloseHandle(TempSemahpore);
}

/*
Description:	This function create the sort thread

Arguments:		ParamStruct *param		- a pointer to the parameter of the sort thread
				LPDWORD *p_thread_id	- a pointer to the id Handle of the sort thread
				HANDLE *SortThread		- a pointer to the sort thread handle
*/

void CreateSortThread( ParamStruct *param, LPDWORD *p_thread_id, HANDLE *SortThread)
{
	if (param == NULL)
	{
		printf("An error occured during the ParamStruct memory allocation.\n Exiting program...\n");
		exit(-1);
	}
	*SortThread = CreateThreadSimple(SortListThread, p_thread_id, param);
}
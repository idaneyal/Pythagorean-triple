
/*
Authors:		Idan Eyal

Project:		this project purpose is to find primitive Pythagorean tiriples with threads

Arguments:		This project get a file as an argument. the file must contain the follow parameters in this order:
			- positive max pythagorean number 
			- integer number of threads (0<n<=100)
			- positive integer Buffer size
			- output file name

Description:	This is the main file of the project.
*/

#include "Threads.h"
#include "Pythagorean.h"



int CreateMutexVaild(int num, int **MutexValid);
void FreeAll(int CalculationThreadsSize, HANDLE MutexArr[], int MutexValid[], LPCSTR EmptySemaphore, LPCSTR FullSemaphore, LPCSTR BufferMutexName,
	PythagoreanTriple *ListHead, PythagoreanTriple *Buffer, LPDWORD ID[], ParamStruct *param, HANDLE ThreadArr[], int MaxNumber);
void CheckIfThreadsEnded(HANDLE SortThread, HANDLE ThreadsArr[], int Size);
int OpenThreadsArray(int Number, HANDLE **ThreadsHandleArr, LPDWORD **p_thread_id, ParamStruct *param);
void IntilizeParam(ParamStruct *param, int MaxNumber);


int main(int argc, char *argv[])
{
	HANDLE *ThreadsArr = NULL, *MutexArr = NULL, SortThread;
	LPDWORD *ThreadsIDArray = NULL, SortId = NULL;
	int NumOfThreads=atoi(argv[2]);
	int BufferSize=atoi(argv[3]);
	int *MutexValid = NULL, MaxNumber;
	LPCSTR EmptyCells = _T("PlacesFree"), FullCells = _T("PlacesTaken"), BufferMutexName= _T("BufferMutex");
	PythagoreanTriple *ListHead = NULL, *Buffer = NULL;
	ParamStruct param;
	MaxNumber = atoi(argv[1]);

	/*This section Create the threads handle array and the MutexValid array*/
	if (CreateMutexArr(MaxNumber,&MutexArr) == FAIL || CreateMutexVaild(MaxNumber,&MutexValid)==FAIL)
	{
		FreeAll(NumOfThreads, MutexArr, MutexValid, EmptyCells, FullCells, BufferMutexName, 
			ListHead, Buffer, ThreadsIDArray, &param, ThreadsArr, MaxNumber);
		return FAIL;
	}

	/*This section intilize the static parameters of the Pythagorean.c file and open the mutex and semaphores*/
	IntilizeParam(&param, MaxNumber);
	UpdateStaticParameters(&MutexArr, &MutexValid, MaxNumber, &Buffer, &ListHead, NumOfThreads);
	OpenMutexAndSemaphore(BufferMutexName, BufferSize);

	/*This section create all the threads*/
	if (OpenThreadsArray(NumOfThreads, &ThreadsArr, &ThreadsIDArray, &param) == FAIL)
	{
		FreeAll(NumOfThreads, MutexArr, MutexValid, EmptyCells, FullCells, BufferMutexName,
			ListHead, Buffer, ThreadsIDArray, &param, ThreadsArr, MaxNumber);
		return FAIL;
	}	
	CreateSortThread(&param, &SortId, &SortThread);

	/*This section print the result, close the handles and free all the memory*/
	CheckIfThreadsEnded(SortThread, ThreadsArr, NumOfThreads);
	PrintList(argv[4]);
	FreeAll(NumOfThreads, MutexArr, MutexValid, EmptyCells, FullCells, BufferMutexName,
		ListHead, Buffer, ThreadsIDArray, &param, ThreadsArr, MaxNumber);
	return SUCCESS;
}

/*
Description:	This function create an integer array which indicate if there has been a use of a number as a base.
				The array's value is set as TRUE for a default value

Arguments:		int size			- the size of the array which will be create
				int **MutexValid	- a pointer to mutexvalid array which indicate if a mutex has been used in order to 
									  synchronize the calculation threads

Return:			The function return FAIL if an error occured during the memory allocation
				The function return SUCCESS otherwise
*/

int CreateMutexVaild(int Size, int **MutexValid)
{
	int i ;
	*MutexValid = (int*)malloc(Size * sizeof(int));
	if (*MutexValid == NULL)
	{
		printf("An error occured during the memory allocation of MutexValid array\n"
			"Exiting program...\n");
		return FAIL;
	}
	for (i = 0; i < Size; i++)
	{
		(*MutexValid)[i] = TRUE;
	}
	return SUCCESS;
}

/*
Description:	This function release all the threads, mutexes, semaphores and memory which have been allocated for the program

Arguments:		int CalculationThreadsSize											- the number of calculation threads
				HANDLE MutexArr[]													- the handle of the mutex of the buffer
				int MutexValid[]													- the mutex is valid aaray
				LPCSTR EmptySemaphore, LPCSTR FullSemaphore, LPCSTR BufferMutexName	- the LPCSTR of the semaphores and the buffer mutex
				PythagoreanTriple *ListHead											- a pointer to the final list of the pythagorean triples
				PythagoreanTriple *Buffer											- a pointer to the buffer
				LPDWORD ID[]														- an array of the threads id
				ParamStruct *param													- a pointer to the parameters which the threads recived
				HANDLE ThreadArr[]													- an array of the calculation threads handle
				int MaxNumber														- the max number argument as an integer
*/

void FreeAll(int CalculationThreadsSize, HANDLE MutexArr[], int MutexValid[], LPCSTR EmptySemaphore, LPCSTR FullSemaphore, LPCSTR BufferMutexName,
	PythagoreanTriple *ListHead, PythagoreanTriple *Buffer, LPDWORD ID[], ParamStruct *param, HANDLE ThreadArr[], int MaxNumber)
{
	CloseAllHandles(MutexArr, MaxNumber, EmptySemaphore, FullSemaphore, BufferMutexName, ThreadArr,CalculationThreadsSize);
	FreeList(ListHead);
	FreeList(Buffer);
	if (ThreadArr != NULL)
		free(ThreadArr);
	if (MutexArr != NULL)
		free(MutexArr);
	if (MutexValid != NULL)
		free(MutexValid);
	if (ID != NULL)
		free(ID);
	if (param->ThreadsFinish != NULL)
		free(param->ThreadsFinish);

}

/*
Description:	This function Check if all the threads have ended and close the handles

Arguments:		HANDLE SortThread	- the Handle of the sort thread
				HANDLE ThreadsArr[]	- the Handle array of the calculation threads
				int Size			- the size of the calculation threads array

Algorithm:		for loop	
					wait for each of the calculation threads to end
					check if wait code or exit code are not as expected(an error occured)
						exit program
				check the sort thread

*/

void CheckIfThreadsEnded(HANDLE SortThread, HANDLE ThreadsArr[], int Size)
{
	int i;
	DWORD waitcode;
	DWORD exitcode = 0;
	for (i = 0; i < Size; i++)
	{
		waitcode = WaitForSingleObject(ThreadsArr[i], INFINITE);
		CloseHandle(ThreadsArr[i]);
		GetExitCodeThread(ThreadsArr[i], &exitcode);
		
	}
	waitcode = WaitForSingleObject(SortThread, INFINITE);
	CloseHandle(SortThread);
	GetExitCodeThread(SortThread, &exitcode);
	if (waitcode != WAIT_OBJECT_0 || exitcode != 0)
		printf("An Error occured in sort thread\n");
}

/*
Description:	This function create a thread array and open the threads

Arguments:		int Number					- the number of calculation threads
				HANDLE **ThreadsHandleArr	- A pointer to the threads Handler array
				LPVOID param[]				- An array of parameters which will transfer to the threads
				LPDWORD **p_thread_id		- An array of threads id

Returns:		The function return FAIL if an error occured during the memory allocation
				The function return Success if it worked without errors
*/

int OpenThreadsArray(int Number, HANDLE **ThreadsHandleArr, LPDWORD **p_thread_id, ParamStruct *param)
{
	int i;
	HANDLE temp;
	*ThreadsHandleArr = (HANDLE*)malloc(Number * sizeof(HANDLE));
	*p_thread_id = (LPDWORD*)malloc(Number * sizeof(LPDWORD));
	if (*p_thread_id == NULL || ThreadsHandleArr == NULL)
	{
		printf("And error occured during the creation of the threads.\n Exiting Program...\n");
		return FAIL;
	}
	for (i = 0; i < Number; i++)
	{
		(*p_thread_id)[i] = NULL;
		temp = CreateThreadSimple(FindPythagoreanTriple, *p_thread_id + i, param);
		(*ThreadsHandleArr)[i] = temp;
	}

	return SUCCESS;
}

/*
Description:	This function intilize the parameter which is sent to the threads. the parameter is an array which indicate if each base (n)
				has ended it calculation

Arguments:		ParamStruct *param	- the parameter which will be send to the threads
				int MaxNumber		- the value of the highest number which need to be calculate in the program


*/

void IntilizeParam(ParamStruct *param, int MaxNumber)
{
	int i;
	param->ThreadsFinish = (BOOL*)malloc(MaxNumber * sizeof(BOOL));
	if (param->ThreadsFinish == NULL)
	{
		printf("An Error occured during the creation of the param struct.\n Exiting program...\n");
		exit(-1);
	}

	//intilizing the value of the array to FALSE which indicate the base still need to be compute
	for (i = 0; i < MaxNumber; i++)
	{
		(param->ThreadsFinish)[i] = FALSE;
	}
}

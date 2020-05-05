/*
Description:	This modul is in chagre of the calculation of the pythagoream triples
				the functions here have 3 purposes: to calculate the pythagoream triples, to insert them to and extract them frim
				the buffer and to sort the triples
*/

#include "Pythagorean.h"

static HANDLE *MutexArr = NULL;
static int *MutexValid = NULL;
static BOOL *NumbersHaveFinish = NULL;
static int MaxNumber = 0;
static HANDLE BufferMutex;
static HANDLE Empty;
static HANDLE Full;
static LPCSTR EmptyCells = _T("PlacesFree");
static LPCSTR FullCells = _T("PlacesTaken");
static PythagoreanTriple *Buffer = NULL;
static PythagoreanTriple *ListHead = NULL;


/*
Description:	This function check if n and m are coprime numbers

Argumnets:		int m		- the first number
				int n		- the second number

Algorithm:		if both numbers are even or odd
					return FAIL
				for loop
					check if there is a number which both numbers can be divided by
						if there is return FAIL
				return SUCCESS


Returns:		SUCCESS - the value 0 if the function succeed
				Fail	- the value -1 if an error occured during the function operation
*/

int CheckIfCoprimeNumbers(int m, int n)
{
	int i;
	if ((m % 2) == (n % 2))
	{
		return FAIL;
	}
	for (i = 3; i <= n; i++)
	{
		if ((n%i) == 0 && (m%i) == 0)
		{
			return FAIL;
		}
	}
	return SUCCESS;

}

/*
Description:	This function calculate the pythagorean numbers

Argumnets:		int m				- the first number of the coprime numbers
				int n				- the second number of the coprime numbers
				int *FirstNumber	- the first number of the pythagorean numbers
				int *SecondNumber	- the second number of the pythagorean numbers
				int *ThridNumber	- the third number of the pythagorean numbers

*/

void CalculatePythagoreanNumbers(int m, int n, int *FirstNumber, int *SecondNumber, int *ThridNumber)
{
	*FirstNumber = (m*m) - (n*n);
	*SecondNumber = 2 * m*n;
	*ThridNumber = (m*m) + (n*n);

}

/*
Description:	This function create a PythagoreanTriple struct and fill it with the value of m, n and 
				the pythagorean Triple number

Arguments:		int m	- the first number of the coprime numbers
				int n	- the second number of the coprime numbers

Algorithm:		create a new member of PythagoreanTriple struct
				fill m and n
				clculate the Pythagorean Triple number
				fill the number

Returns:		PythagoreanTriple *NewMember if the function succeed
				NULL if an error occured during the memory allocation
*/

PythagoreanTriple* CreatePythagorean(int m, int n)
{
	PythagoreanTriple *NewMember = NULL;
	int a = 0, b = 0, c = 0;
	NewMember = (PythagoreanTriple*)malloc(sizeof(PythagoreanTriple));
	if (NewMember == NULL)
	{
		printf("An error occured during the creation of a PythagoreanTriple"
			"Exiting program...\n");
		return NULL;
	}
	NewMember->m = m;
	NewMember->n = n;
	CalculatePythagoreanNumbers(m, n, &a, &b, &c);
	NewMember->a = a;
	NewMember->b = b;
	NewMember->c = c;
	NewMember->next = NULL;
	return NewMember;
}

/*
Description:	This function get a new member and add it to the sorted list

Arguments:		PythagoreanTriple *ListHead	- a pointer to the head of the list
				PythagoreanTriple *Member	- a pointer to the member

Algorithm:		if the list is empty
					return the new member
				find the location in the list of the new member
				if the location is NULL
					the member will become the head of the list
				update the list
				return listhead

Returns:		The function a pointer of type Pythagorean struct which point on the New list head (the listhead not  necessarily change)
*/

PythagoreanTriple* AddMemberToList(PythagoreanTriple *ListHead,PythagoreanTriple *Member)
{
	int n, m;
	PythagoreanTriple  *temp = NULL;
	if (ListHead == NULL){
		return Member;
	}
	n = Member->n;
	m = Member->m;
	temp = FindMemberLocation(m, n, ListHead);
	if (temp == NULL){
		Member->next = ListHead;
		return Member;
	}
	Member->next = temp->next;
	temp->next = Member;
	return ListHead;

}

/*
Description:	This function find the member in the list which the new member should apear after

Arguments:		int m						- the first number of the coprime numbers
				int n						- the second number of the coprime numbers
				PythagoreanTriple *ListHead	- a pointer to the head of the list

Algorithm:		while loop if there are still members in the list
					if the current member's n value is higher than the n value of the new member
						the New member should appear after the previous member
					if the current member's n is equal to the n value of the new member
						if the current member's m value is higher than the m value of the new member
							the New member should appear after the previous member
					if there was not match this iteration continue to the next member in the list and update prev
				if the loop ended the new member should be the tail

Returns:		a pointer to the member which should apear before the new member in the list
				NULL if the new member should be the new head of the list
*/

PythagoreanTriple* FindMemberLocation(int m, int n, PythagoreanTriple *ListHead)
{
	PythagoreanTriple *prev = NULL;
	while (ListHead != NULL){
		if (ListHead->n > n){
			return prev;
		}
		if (ListHead->n == n){
			if (ListHead->m > m){
				return prev;
			}
		}
		prev = ListHead;
		ListHead = ListHead->next;		
	}
	return prev;
}

/*
Description:	This function check if a thread can use a number as a base to claculate the pythagorean triple from it

Arguments:		int Number			- The number which we want to check

Algorithm:		Check if the requested mutex is used currently
					if it is return false
				else
					if Mutex is valid (it haven't been used before)
						the current mutex is signaled as used(FALSE)
						return TRUE
					else
						the mutex was used before
						return FALSE

Returns:		The function return TRUE if the thread can use the current number
				The function return FALSE if it can't use the current number
*/

BOOL CheckIfCanAccessNumber(int Number)
{
	DWORD waitcode;
	BOOL RetVal;
	waitcode = WaitForSingleObject(MutexArr[Number], 0);
	if (waitcode != WAIT_OBJECT_0)
	{
		return FALSE;
	}
	else
	{
		if (MutexValid[Number] == TRUE)
		{
			MutexValid[Number] = FALSE;
			RetVal = ReleaseMutex(MutexArr[Number]);
			if (RetVal == FALSE)
			{
				printf("An error occured during the release of the mutex...\n"
					"Exiting program\n");
				exit(-1);
			}
			return TRUE;
		}
		else
		{
			RetVal = ReleaseMutex(MutexArr[Number]);
			if (RetVal == FALSE)
			{
				printf("An error occured during the release of the mutex...\n"
					"Exiting program\n");
				exit(-1);
			}
			return FALSE;
		}
	}
}

/*
Description:	This function create all the pythagorean triples for a specific base and insert them to the bufffer

Arguments:		int Number		- the base (n) of the pythagorean triples which we try to find

Algorithm:		for loop
					check if each m and Number are coprime numbers
						create a new pythagorean member
						insert it to the buffer
*/

void FindTriples(int Number)
{
	int m;
	PythagoreanTriple *member;
	for (m = Number + 1; m <= MaxNumber; m++)
	{
		if (CheckIfCoprimeNumbers(m, Number) == SUCCESS)
		{
			member = CreatePythagorean(m, Number);
			if (member == NULL)
			{
				exit(-1);
			}
			OpenInsertBufferGuards();
			InsertToBuffer(member);
			CloseInsertBufferGuards();
		}
	}
}

/*
Description:	This function is the main function of the calculation threads. It in charge of calculation all the pythagorean triples.

Arguments:		ParamStruct *param	- a pointer to the parameter which contain the ThreadsFinish array

Algorithm:		for loop
					check if the current n number is accessable and wasn't used
						find all the tripls which n is their base
						update ThreadFinish array at the location of the number to be TRUE(indicate that all the pythagorean triples with 
																							that base have been found)
*/

void FindAllTriples(ParamStruct *param)
{
	int n;
	for (n = 1; n <= MaxNumber; n++)
	{
		if (CheckIfCanAccessNumber(n-1) == TRUE)
		{
			FindTriples(n);
			(param->ThreadsFinish)[n-1] = TRUE;
		}
	}
	
}

/*
Descriotion:	This function insert the new Pythagorean member to the end of the buffer

Arguments:		PythagoreanTriple *member	- The new member which we want to insert to the buffer

algorithm:		if the Buffer is empty
					the buffer will contain only the new member
				member become the new head of the list

*/

void InsertToBuffer(PythagoreanTriple *member)
{
	if (Buffer == NULL)
	{
		Buffer = member;
		return;
	}
	member->next = Buffer;
	Buffer = member;
}

/*
Description:	This function check when the thread have permission to access the buffer before inserting the new member

Algorithm:		wait until there are empty cells in the buffer
				wait until the buffer mutex is usable

*/

void OpenInsertBufferGuards()
{
	DWORD waitcode = 0;
	waitcode = WaitForSingleObject(Empty, INFINITE);
	if (waitcode != WAIT_OBJECT_0) {
		exit(-1);
	}
	waitcode = WaitForSingleObject(BufferMutex, INFINITE);
	if (waitcode != WAIT_OBJECT_0) {
		printf("An Error occured during waiting to the buffer mutex.\n Exiting Thread\n");
		exit(-1);
	}
}

/*
Description: This function release the mutex and semaphore of the buffer after inserting the new member
*/

void CloseInsertBufferGuards()
{
	if (ReleaseMutex(BufferMutex) == FALSE)
	{
		printf("An Error occured during releasing to the buffer mutex.\n Exiting Thread\n");
		exit(-1);
	}
	if (ReleaseSemaphore(Full, 1, NULL) == FALSE)
	{
		printf("An Error occured during releasing to the buffer's full semaphore.\n Exiting Thread\n");
		exit(-1);
	}
}

/*
Description:	This is the main function of the sort thread

Arguments:		ParamStruct *param	- a pointer to the parameter which the function recieve.

Algorithm:		while loop
					check if need to break the loop and exit the function(the thread has ended)
					if can access the buffer(wait for the buffer mutex and semaphore)
						extract a new member from the start of the buffer
						release the mutex and semaphore
						if the new mwmber is NULL
							and error occured so exit thread
						add the new member to the list
						continue to the next iteration
					else
						continue to the next iteration
				free NumberHaveFinish array
*/

void SortList(ParamStruct *param)
{
	PythagoreanTriple *member = NULL;
	
	while (1)
	{
		if (CheckIfBreak(param) == TRUE)
		{
			return;
		}
		if (OpenExtractBufferGuards() == TRUE)
		{
			member = Buffer;
			Buffer = Buffer->next;
			CloseExtractBufferGuards();
			if (member == NULL)
			{
				printf("The member which have been extract from the buffer is NULL.\n"
					"Exiting program...\n");
				exit(-1);
			}
			member->next = NULL;
			ListHead = AddMemberToList(ListHead, member);
		}
	}
	free(NumbersHaveFinish);
}

/*
Description:	This function check each iteration if the sort thread ended and need to exit it

Arguments:		ParamStruct *param	- a pointer to the parameter of the function which contain the pointer to the ThreadFinish Array
									  which indicate if the calculation threads has ended to calculate all the members

Algorithm:		for each number between 0 to MaxNumber-1
					check if the current value of ThreadFinish is FALSE (the base still calculate new members)
						return FALSE 
				all the calculation threads ended
				if there are not any members in the buffer
					return TRUE
				return FALSE

Returns:		FALSE- mean the sort thread need to continue because the calculation threads still have'nt finished or there are still members in the buffer
				TRUE - mean the sort thread has finish to sort all the pythagorean triples and the calculation threads won't produce new members
*/

BOOL CheckIfBreak(ParamStruct *param)
{
	int i;
	for (i = 0; i < MaxNumber; i++)
	{
		if ((param->ThreadsFinish)[i] == FALSE)
		{
			return FALSE;
		}
	}
	if (Buffer == NULL)
	{
		return TRUE;
	}
	return FALSE;
}

/*
Description:	This function check when the sorting thread have permission to access the buffer before extracting a member from the buffer

Algorithm:		wait WAIT_TIME_FOR_SEMAPHORE seconds for semaphore Full to be free
				if it is not free WAIT_TIME_FOR_SEMAPHORE has elapsed
					return FALSE
					the wait time is not INFINITE in order to prevent deadlocks
				wait INFINITE to buffer mutex to be free
				return TRUE

Returns:		FALSE	- when the wait time of the Full semaphore elapsed. this value signal to loop of the previous function to check if
						  the calculation threads has ended and therfore prevent a deadlock.
				TRUE	- can access the buffer
*/

BOOL OpenExtractBufferGuards()
{
	DWORD waitcode = 0;
	waitcode = WaitForSingleObject(Full, WAIT_TIME_FOR_SEMAPHORE);
	if (waitcode != WAIT_OBJECT_0) {
		return FALSE;
	}
	waitcode = WaitForSingleObject(BufferMutex, INFINITE);
	if (waitcode != WAIT_OBJECT_0) {
		printf("An Error occured during waiting to the buffer mutex.\n Exiting Thread\n");
		exit(-1);
	}
	return TRUE;
}

/*
Description: This function release the mutex and semaphore of the buffer after extracting a member from the buffer
*/

void CloseExtractBufferGuards()
{
	if (ReleaseMutex(BufferMutex) == FALSE)
	{
		printf("An Error occured during releasing to the buffer mutex.\n Exiting Thread\n");
		exit(-1);
	}
	if (ReleaseSemaphore(Empty, 1, NULL) == FALSE)
	{
		printf("An Error occured during releasing to the buffer's empty semaphore.\n Exiting Thread\n");
		exit(-1);
	}
}

/*
Description:	This function intilize the static parameters of the Pythagorean module so all the function could use them.

Arguments:		HANDLE **MArr					- a pointer to the mutex array which indicate if a base is accessable
				int **VArr						- a pointer to Valid array which indicate if the base has been calculated by a thread
				int maxnumber					- the max number argument
				PythagoreanTriple **buffer		- a pointer to the buffer
				PythagoreanTriple **listhead	- a pointer to the list
*/

void UpdateStaticParameters(HANDLE **MArr, int **VArr, int maxnumber, PythagoreanTriple **buffer, PythagoreanTriple **listhead)
{
	int i;
	MutexArr = *MArr;
	MutexValid = *VArr;
	MaxNumber =maxnumber;
	Buffer = *buffer;
	ListHead = *listhead;
	NumbersHaveFinish = (BOOL*)malloc(maxnumber * sizeof(BOOL));
	if (NumbersHaveFinish == NULL)
	{
		printf("An error occured during the creation of NumbersHaveFinish memory allocation\n"
			"Exiting program...\n");
		exit(-1);
	}
	for (i = 0; i < maxnumber; i++)
		NumbersHaveFinish[i] = FALSE;
}

/*
Description:	This function print the final sorted list of the pythagoream numbers and free the list members

Arguments:		char FileName[]	- the name of the output file
*/

void PrintList(char FileName[])
{
	PythagoreanTriple *temp = NULL;
	FILE *fp = fopen(FileName, "w");
	if (fp == NULL)
	{
		exit(-1);
	}
	while (ListHead != NULL)
	{
		fprintf(fp,"%d,%d,%d\n", ListHead->a, ListHead->b, ListHead->c);
		temp = ListHead;
		ListHead = ListHead->next;
		free(temp);
	}
	fclose(fp);
}

/*
Description:	This function create the semaphore and the mutex buffer

Arguments:		LPCSTR BufferMutexName	- the buffer mutex name
				int BufferSize			- the size of the buffer
*/

void OpenMutexAndSemaphore(LPCSTR BufferMutexName, int BufferSize)
{
	BufferMutex = CreateMutex(NULL, FALSE, BufferMutexName);
	CreateSemaphoreSimple(BufferSize, EmptyCells, FullCells, &Full, &Empty);
}

/*
Description:	This function free a linked list of Pythagorean struct

Arguments:		PythagoreanTriple *List	- the list which need to be free
*/

void FreeList(PythagoreanTriple *List)
{
	PythagoreanTriple *temp = NULL;
	while (List != NULL)
	{
		temp = List;
		List = List->next;
		free(temp);
	}
}
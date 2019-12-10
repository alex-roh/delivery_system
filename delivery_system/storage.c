#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "storage.h"

/*
  definition of storage cell structure ----
  members :
  int building : building number of the destination
  int room : room number of the destination
  int cnt : number of packages in the cell
  char passwd[] : password setting (4 characters)
  char *contents : package context (message string)
*/
typedef struct {
	int building;	// ������ ����
	int room;		// ������ ����
	int cnt;		// ����� �ȿ� �ִ� ��Ű���� �� (�⺻: 0)

	char passwd[PASSWD_LEN + 1]; // �н�����

	char* context;	// �ù� ���빰
} storage_t;

static storage_t** deliverySystem; 			//deliverySystem
static int storedCnt = 0;					//number of cells occupied
static int systemSize[2] = { 0, 0 };  		//row/column of the delivery system
static char masterPassword[PASSWD_LEN + 1];	//master password

// ------- inner functions ---------------

//print the inside context of a specific cell
//int x, int y : cell to print the context
static void printStorageInside(int x, int y) {
	printf("\n------------------------------------------------------------------------\n");
	printf("------------------------------------------------------------------------\n");
	if (deliverySystem[x][y].cnt > 0)
		printf("<<<<<<<<<<<<<<<<<<<<<<<< : %s >>>>>>>>>>>>>>>>>>>>>>>>>>>>\n", deliverySystem[x][y].context);
	else
		printf("<<<<<<<<<<<<<<<<<<<<<<<< empty >>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

	printf("------------------------------------------------------------------------\n");
	printf("------------------------------------------------------------------------\n\n");
}

// ó�� ���� ������ �ý����� ���� ��, �����ͺ��̽��κ��� �ε��ϴ� �Լ� 
static void loadFromDatabase(char* filepath)
{
	int row = 0, column = 0;
	FILE* fp;
	fp = fopen(filepath, "r");
	char temp[MAX_MSG_SIZE];

	// ù��° ��� �� ��° ���� ���� ������ �ý��۰� ���������Ƿ� skip
	fgets(temp, sizeof(temp), fp);
	fgets(temp, sizeof(temp), fp);

	// �����ͺ��̽��� Ž��
	while (1)
	{
		char buffer[MAX_MSG_SIZE];

		// �����ͺ��̽��κ��� �� ������ �о��
		fgets(buffer, sizeof(buffer), fp);
			
		// ������ ���� ������ ���
		if (feof(fp))
		{
			fclose(fp);
			return;
		}

		// ������ �������� ������ ����
		char* ptr = strtok(buffer, " ");

		// ���� ����
		row = atoi(ptr);
		
		// ���� ����
		ptr = strtok(NULL, " ");
		column = atoi(ptr);

		// ������ �ǹ�
		ptr = strtok(NULL, " ");
		deliverySystem[row][column].building = atoi(ptr);

		// ������ ȣ��
		ptr = strtok(NULL, " ");
		deliverySystem[row][column].room = atoi(ptr);

		// ��й�ȣ
		ptr = strtok(NULL, " ");
		strcpy(deliverySystem[row][column].passwd, ptr);

		// ���빰
		ptr = strtok(NULL, "\n");
		deliverySystem[row][column].context = (char*)malloc(sizeof(char) * MAX_MSG_SIZE);
		strcpy(deliverySystem[row][column].context, ptr);

		// ���빰�� ���� (cnt)
		deliverySystem[row][column].cnt = 1;
	}

}

//initialize the storage
//set all the member variable as an initial value
//and allocate memory to the context pointer
//int x, int y : cell coordinate to be initialized
static void initStorage(int x, int y) {

	// ���빰�� ������ 1�� �ƴ� ���� �ʱ�ȭ
	if (deliverySystem[x][y].cnt != 1)
	{
		deliverySystem[x][y].building = 0;
		deliverySystem[x][y].room = 0;
		deliverySystem[x][y].cnt = 0;
		strcpy(deliverySystem[x][y].passwd, "0");
		deliverySystem[x][y].context = (char*)malloc(sizeof(char) * MAX_MSG_SIZE);
		strcpy(deliverySystem[x][y].context, "0");
	}
}

//get password input and check if it is correct for the cell (x,y)
//int x, int y : cell for password check
//return : 0 - password is matching, -1 - password is not matching
static int inputPasswd(int x, int y) {

}

// ------- API function for main.c file ---------------

//backup the delivery system context to the file system
//char* filepath : filepath and name to write
//return : 0 - backup was successfully done, -1 - failed to backup
int str_backupSystem(char* filepath) {

}

//create delivery system on the double pointer deliverySystem
//char* filepath : filepath and name to read config parameters
// (row, column, master password, past contexts of the delivery system)
//return : 0 - successfully created, -1 - failed to create the system
int str_createSystem(char* filepath) {

	int i = 0, j = 0, lineCnt = 0, alloc_flag = 0;
	FILE* fp;
	char buffer[MAX_MSG_SIZE];

	fp = fopen(filepath, "r");

	if (fp == NULL)
	{
		return -1;
	}

	fgets(buffer, sizeof(buffer), fp); // ���ڿ� �� ���� �޾ƿ�

	// �ý����� ��� ���� ����
	char* ptr = strtok(buffer, " ");
	systemSize[0] = atoi(ptr); // ��      // ���⼭ ���� �߻�
	ptr = strtok(NULL, "\n");
	systemSize[1] = atoi(ptr); // ��

	fclose(fp);

	// ���� ������ �ý��ۿ� �޸� ���� �Ҵ�
	deliverySystem = (storage_t**)malloc(sizeof(storage_t*) * systemSize[0]);
	for (i = 0; i < systemSize[0]; i++)
	{
		deliverySystem[i] = (storage_t*)malloc(sizeof(storage_t) * systemSize[1]);
	}

	// �����ͺ��̽����� �ε�
	loadFromDatabase(filepath);

	// ���� ������ �ý����� �ʱ�ȭ
	for (i = 0; i < systemSize[0]; i++)
	{
		for (j = 0; j < systemSize[1]; j++)
		{
			initStorage(i, j);
		}
	}

	return 0;
}

//free the memory of the deliverySystem 
void str_freeSystem(void) {

}

//print the current state of the whole delivery system (which cells are occupied and the destination of the each occupied cells)
void str_printStorageStatus(void) {
	int i, j;
	printf("----------------------------- Delivery Storage System Status (%i occupied out of %i )-----------------------------\n\n", storedCnt, systemSize[0] * systemSize[1]);

	printf("\t");

	for (j = 0; j < systemSize[1]; j++)
	{
		printf(" %i\t\t", j);
	}
	printf("\n-----------------------------------------------------------------------------------------------------------------\n");

	for (i = 0; i < systemSize[0]; i++)
	{
		printf("%i|\t", i);
		for (j = 0; j < systemSize[1]; j++)
		{
			if (deliverySystem[i][j].cnt > 0)
			{
				printf("%i,%i\t|\t", deliverySystem[i][j].building, deliverySystem[i][j].room);
			}
			else
			{
				printf(" -  \t|\t");
			}
		}
		printf("\n");
	}
	printf("--------------------------------------- Delivery Storage System Status --------------------------------------------\n\n");
}

//check if the input cell (x,y) is valid and whether it is occupied or not
int str_checkStorage(int x, int y) {
	if (x < 0 || x >= systemSize[0])
	{
		return -1;
	}

	if (y < 0 || y >= systemSize[1])
	{
		return -1;
	}

	return deliverySystem[x][y].cnt;
}


//put a package (msg) to the cell
//input parameters
//int x, int y : coordinate of the cell to put the package
//int nBuilding, int nRoom : building and room numbers of the destination
//char msg[] : package context (message string)
//char passwd[] : password string (4 characters)
//return : 0 - successfully put the package, -1 - failed to put
int str_pushToStorage(int x, int y, int nBuilding, int nRoom, char msg[MAX_MSG_SIZE + 1], char passwd[PASSWD_LEN + 1]) {

	// ���� ��Ű���� ����ִ� �Լ�

}

//extract the package context with password checking
//after password checking, then put the msg string on the screen and re-initialize the storage
//int x, int y : coordinate of the cell to extract
//return : 0 - successfully extracted, -1 = failed to extract
int str_extractStorage(int x, int y) {

	// ������ ��Ű���� �����ϴ� �Լ�

}

//find my package from the storage
//print all the cells (x,y) which has my package
//int nBuilding, int nRoom : my building/room numbers
//return : number of packages that the storage system has
int str_findStorage(int nBuilding, int nRoom) {



	return 0;
}
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

		// ��ü ����� ���빰 ���� ������Ʈ
		storedCnt++;
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

	char passwd[PASSWD_LEN + 1];

	printf(" - input password for (%d, %d) storage: ", x, y);
	scanf("%s", passwd);
	fflush(stdin);

	// ������ �н����带 �Է��ϰų�, �ùٸ� �н����带 �Է��� ���
	if (strcmp(masterPassword, passwd) == 0
		|| strcmp(deliverySystem[x][y].passwd, passwd) == 0)
	{
		return 0;
	}
	// Ʋ�� �н����带 �Է��� ���
	else 
	{
		return -1;
	}
}

// ------- API function for main.c file ---------------

//backup the delivery system context to the file system
//char* filepath : filepath and name to write
//return : 0 - backup was successfully done, -1 - failed to backup
int str_backupSystem(char* filepath) {
	
	int i = 0, j = 0;
	FILE* fp;
	char systemRowCol[10];
	char temp_masterPassword[10];

	// �ؽ�Ʈ ������ ���� ������ ���� "w" ���� ����
	fp = fopen(filepath, "w");
	if (fp == NULL)
	{
		return -1;
	}

	// �ý����� ��� ���� �����ͺ��̽��� �Է�
	sprintf(systemRowCol, "%d %d\n", systemSize[0], systemSize[1]);
	fputs(systemRowCol, fp);

	// �ý����� ������ �н����带 �����ͺ��̽��� �Է�
	fputs(masterPassword, fp);

	// �ý����� ���� ���¸� ������Ʈ
	for (i = 0; i < systemSize[0]; i++)
	{
		for (j = 0; j < systemSize[1]; j++)
		{
			if (deliverySystem[i][j].cnt == 1)
			{
				char temp[MAX_MSG_SIZE];
				sprintf(temp, "%d %d %d %d %s %s\n", i, j,
					deliverySystem[i][j].building,
					deliverySystem[i][j].room,
					deliverySystem[i][j].passwd,
					deliverySystem[i][j].context);
				fputs(temp, fp);
			}
		}
	}

	fclose(fp);
	return 0;
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

	// �ý����� ������ �н����带 ����
	fgets(buffer, sizeof(buffer), fp);
	strcpy(masterPassword, buffer);

	fclose(fp); // ���� �����͸� ����

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

	int i;

	for (i = 0; i < systemSize[0]; i++)
	{
		free(deliverySystem[i]);
	}

	free(deliverySystem);

}

//print the current state of the whole delivery system (which cells are occupied and the destination of the each occupied cells)
void str_printStorageStatus(void) {
	int i, j;
	printf("----------------------------- Delivery Storage System Status (%i occupied out of %i )-----------------------------\n\n",
		storedCnt, systemSize[0] * systemSize[1]);

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

	// �ְ��� �ϴ� ���� �̹� �� �ִ� ���
	if (deliverySystem[x][y].cnt == 1)
	{
		return -1;
	}

	// ���� ��Ű���� ����ִ� �Լ�
	deliverySystem[x][y].building = nBuilding;
	deliverySystem[x][y].room = nRoom;
	deliverySystem[x][y].cnt = 1;
	strcpy(deliverySystem[x][y].passwd, passwd);
	strcpy(deliverySystem[x][y].context, msg);

	// ��ü ���빰�� ������ 1 ������Ŵ
	storedCnt++;

	fflush(stdin);

	return 0;
}

//extract the package context with password checking
//after password checking, then put the msg string on the screen and re-initialize the storage
//int x, int y : coordinate of the cell to extract
//return : 0 - successfully extracted, -1 = failed to extract
int str_extractStorage(int x, int y) {

	// �н����尡 ��ġ�� ������ ��Ű���� ���� ����
	if (inputPasswd(x, y) == 0)
	{
		// �ش� ���� ��Ű���� �ִ� ���
		if (deliverySystem[x][y].cnt == 1)
		{
			printf(" ----------->extracting the storage(%d, %d)...", x, y);
			deliverySystem[x][y].building = 0;
			deliverySystem[x][y].room = 0;
			deliverySystem[x][y].cnt = 0;
			strcpy(deliverySystem[x][y].passwd, "0");
			free(deliverySystem[x][y].context);
			deliverySystem[x][y].context = (char*)malloc(sizeof(char) * MAX_MSG_SIZE);
			strcpy(deliverySystem[x][y].context, "0");

			// ��ü ����� ���빰 ���� ����
			storedCnt--;
		}
		else 
		{
			// �ش� ���� ��Ű���� ���� ���
			return -1;
		}
	}
	// �н����尡 ��ġ���� �ʴ� ���
	else
	{
		return -1;
	}

	return 0;
}

//find my package from the storage
//print all the cells (x,y) which has my package
//int nBuilding, int nRoom : my building/room numbers
//return : number of packages that the storage system has
int str_findStorage(int nBuilding, int nRoom) {

	int i = 0, j = 0, cnt = 0;

	for (i = 0; i < systemSize[0]; i++)
	{
		for (j = 0; j < systemSize[1]; j++)
		{
			if (deliverySystem[i][j].building == nBuilding 
				&& deliverySystem[i][j].room == nRoom)
			{
				printf("----------->Found a package in(%d, %d)\n", i, j);
				cnt++;
			}		
		}
	}

	return cnt;
}
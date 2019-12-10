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
	int building;	// 목적지 빌딩
	int room;		// 목적지 세대
	int cnt;		// 저장소 안에 있는 패키지의 수 (기본: 0)

	char passwd[PASSWD_LEN + 1]; // 패스워드

	char* context;	// 택배 내용물
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

// 처음 무인 보관함 시스템을 만들 때, 데이터베이스로부터 로드하는 함수 
static void loadFromDatabase(char* filepath)
{
	int row = 0, column = 0;
	FILE* fp;
	fp = fopen(filepath, "r");
	char temp[MAX_MSG_SIZE];

	// 첫번째 행과 두 번째 행은 무인 보관함 시스템과 관련있으므로 skip
	fgets(temp, sizeof(temp), fp);
	fgets(temp, sizeof(temp), fp);

	// 데이터베이스를 탐색
	while (1)
	{
		char buffer[MAX_MSG_SIZE];

		// 데이터베이스로부터 한 문장을 읽어옴
		fgets(buffer, sizeof(buffer), fp);

		// 파일의 끝에 도달한 경우
		if (feof(fp))
		{
			fclose(fp);
			return;
		}

		// 공백을 기준으로 문장을 분할
		char* ptr = strtok(buffer, " ");

		// 행을 얻음
		row = atoi(ptr);
		
		// 열을 얻음
		ptr = strtok(NULL, " ");
		column = atoi(ptr);

		// 목적지 건물
		ptr = strtok(NULL, " ");
		deliverySystem[row][column].building = atoi(ptr);

		// 목적지 호수
		ptr = strtok(NULL, " ");
		deliverySystem[row][column].room = atoi(ptr);

		// 비밀번호
		ptr = strtok(NULL, " ");
		strcpy(deliverySystem[row][column].passwd, ptr);

		// 내용물
		ptr = strtok(NULL, "\n");
		deliverySystem[row][column].context = (char*)malloc(sizeof(char) * MAX_MSG_SIZE);
		strcpy(deliverySystem[row][column].context, ptr);

		// 내용물의 개수 (cnt)
		deliverySystem[row][column].cnt = 1;

		// 전체 저장소 내용물 개수 업데이트
		storedCnt++;
	}
}

//initialize the storage
//set all the member variable as an initial value
//and allocate memory to the context pointer
//int x, int y : cell coordinate to be initialized
static void initStorage(int x, int y) {

	// 내용물의 개수가 1이 아닌 셀만 초기화
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

	// 마스터 패스워드를 입력하거나, 올바른 패스워드를 입력한 경우
	if (strcmp(masterPassword, passwd) == 0
		|| strcmp(deliverySystem[x][y].passwd, passwd) == 0)
	{
		return 0;
	}
	// 틀린 패스워드를 입력한 경우
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

	// 텍스트 파일을 새로 덮어씌우기 위해 "w" 모드로 오픈
	fp = fopen(filepath, "w");
	if (fp == NULL)
	{
		return -1;
	}

	// 시스템의 행과 열을 데이터베이스에 입력
	sprintf(systemRowCol, "%d %d\n", systemSize[0], systemSize[1]);
	fputs(systemRowCol, fp);

	// 시스템의 마스터 패스워드를 데이터베이스에 입력
	fputs(masterPassword, fp);

	// 시스템의 현재 상태를 업데이트
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

	fgets(buffer, sizeof(buffer), fp); // 문자열 한 줄을 받아옴

	// 시스템의 행과 열을 결정
	char* ptr = strtok(buffer, " ");
	systemSize[0] = atoi(ptr); // 행      // 여기서 에러 발생
	ptr = strtok(NULL, "\n");
	systemSize[1] = atoi(ptr); // 열

	// 시스템의 마스터 패스워드를 결정
	fgets(buffer, sizeof(buffer), fp);
	strcpy(masterPassword, buffer);

	fclose(fp); // 파일 포인터를 해제

	// 무인 보관함 시스템에 메모리 동적 할당
	deliverySystem = (storage_t**)malloc(sizeof(storage_t*) * systemSize[0]);
	for (i = 0; i < systemSize[0]; i++)
	{
		deliverySystem[i] = (storage_t*)malloc(sizeof(storage_t) * systemSize[1]);
	}

	// 데이터베이스에서 로드
	loadFromDatabase(filepath);

	// 무인 보관함 시스템을 초기화
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

	// 넣고자 하는 셀이 이미 차 있는 경우
	if (deliverySystem[x][y].cnt == 1)
	{
		return -1;
	}

	// 셀에 패키지를 집어넣는 함수
	deliverySystem[x][y].building = nBuilding;
	deliverySystem[x][y].room = nRoom;
	deliverySystem[x][y].cnt = 1;
	strcpy(deliverySystem[x][y].passwd, passwd);
	strcpy(deliverySystem[x][y].context, msg);

	// 전체 내용물의 개수를 1 증가시킴
	storedCnt++;

	fflush(stdin);

	return 0;
}

//extract the package context with password checking
//after password checking, then put the msg string on the screen and re-initialize the storage
//int x, int y : coordinate of the cell to extract
//return : 0 - successfully extracted, -1 = failed to extract
int str_extractStorage(int x, int y) {

	// 패스워드가 일치할 때에만 패키지를 추출 가능
	if (inputPasswd(x, y) == 0)
	{
		// 해당 셀에 패키지가 있는 경우
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

			// 전체 저장소 내용물 개수 감소
			storedCnt--;
		}
		else 
		{
			// 해당 셀에 패키지가 없는 경우
			return -1;
		}
	}
	// 패스워드가 일치하지 않는 경우
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
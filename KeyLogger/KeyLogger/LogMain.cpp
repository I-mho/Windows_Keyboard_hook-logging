#include <stdio.h>
#include <Windows.h>

#define DEF_DLL_NAME "KeyLog.dll"
#define DEF_LOGSTART "LogStart"
#define DEF_LOGSTOP "LogStop"

typedef void (*PFN_LOGSTART)();
typedef void (*PFN_LOGSTOP)();

HMODULE hDll = NULL;
PFN_LOGSTART LogStart = NULL;
PFN_LOGSTOP LogStop = NULL;

BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
    // 강제 종료 신호에 대한 처리
    if (fdwCtrlType == CTRL_C_EVENT || fdwCtrlType == CTRL_CLOSE_EVENT ||
        fdwCtrlType == CTRL_BREAK_EVENT || fdwCtrlType == CTRL_LOGOFF_EVENT ||
        fdwCtrlType == CTRL_SHUTDOWN_EVENT)
    {
        if (LogStop != NULL)
        {
            LogStop();  // 종료 시 LogStop 호출
        }

        if (hDll != NULL)
        {
            FreeLibrary(hDll);  // DLL 해제
        }

        return TRUE; // 신호를 처리했음을 알림
    }
    return FALSE;
}

int main()
{
    // DLL 로드
    hDll = LoadLibraryA(DEF_DLL_NAME);
    if (hDll == NULL)
    {
        printf("DLL 로드 실패: %s\n", DEF_DLL_NAME);
        return 1;
    }

    // 함수 포인터 가져오기
    LogStart = (PFN_LOGSTART)GetProcAddress(hDll, DEF_LOGSTART);
    LogStop = (PFN_LOGSTOP)GetProcAddress(hDll, DEF_LOGSTOP);

    if (LogStart == NULL || LogStop == NULL)
    {
        printf("함수를 가져오는 데 실패했습니다.\n");
        FreeLibrary(hDll);
        return 1;
    }

    // 강제 종료 핸들러 설정
    SetConsoleCtrlHandler(CtrlHandler, TRUE);

    LogStart();  // 로그 시작

    // printf("프로그램이 실행 중입니다. 강제 종료하려면 Ctrl+C를 누르세요.\n");

    // 무한 루프 - q 입력 대기 제거
    while (1)
    {
        Sleep(100); // CPU 사용을 줄이기 위해 잠시 대기
    }

    // 이 부분은 도달하지 않음
    return 0;
}

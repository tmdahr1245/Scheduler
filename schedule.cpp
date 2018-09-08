#include <windows.h>
#include <tchar.h>
#include <winuser.h>
#include <tlhelp32.h>
#include <ctime>
#include <string>
#include <memory>
#include <iostream>
#include <fstream>
#include "json/json.h"
//g++ -o schedule schedule.cpp jsoncpp.cpp -mwindows -municode -std=c++14

#define _UNICODE
#define ICON_ID 8814
#define WM_TRAY 14425
#define MENU_QUIT_MESSAGE 1254
#define ERROR_ALREADY_EXIST 0xB7
using namespace std;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
DWORD32 WINAPI scheduler(LPVOID lpParam);
HANDLE hMutex;
HINSTANCE g_hInst;
LPWSTR lpszClass = TEXT("temp");
string json_file = "schedule.json";
Json::Value root;
Json::Reader reader;
unique_ptr<bool[]> ptr;

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow){
    MSG Message;
    WNDCLASS WndClass;
    g_hInst = hInstance;
    HWND hWnd;
    DWORD dwThreadId = 1;

    ifstream json(json_file, ifstream::binary);
    reader.parse(json, root);
    ptr = make_unique<bool[]>(root["schedule"].size());
    for (int i = 0; i < root["schedule"].size(); i++)
        ptr[i] = false;
    hMutex = CreateMutex(NULL, FALSE, TEXT("mutex"));
    if(GetLastError() == ERROR_ALREADY_EXIST)exit(0);
    
    HANDLE hThreadc = (HANDLE)_beginthreadex(NULL, 0, scheduler, NULL, 0, (DWORD32 *)&dwThreadId);

    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hbrBackground = NULL;
    WndClass.hCursor = NULL;
    WndClass.hIcon = NULL;
    WndClass.hInstance = hInstance;
    WndClass.lpfnWndProc = (WNDPROC)WndProc;
    WndClass.lpszClassName = lpszClass;
    WndClass.lpszMenuName = NULL;
    WndClass.style = CS_HREDRAW | CS_VREDRAW;
    RegisterClass(&WndClass);
    hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW | WS_CAPTION, 0, 0, 500, 500, NULL, (HMENU)NULL, hInstance, NULL);

    while (GetMessage(&Message, 0, 0, 0)){
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }
    return Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
    int wmEvent, wmId;
    HMENU hMenu, hPopupMenu, hMenubar;
    hMenu = CreateMenu();
    hMenubar = CreateMenu();
    AppendMenuW(hMenu, MF_STRING, MENU_QUIT_MESSAGE, TEXT("Quit"));
    AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hMenu, TEXT("File"));
    switch (iMessage){
    case WM_CREATE:
        NOTIFYICONDATA nid;
        ZeroMemory(&nid, sizeof(nid));
        nid.cbSize = sizeof(nid);
        nid.hWnd = hWnd;
        nid.uID = ICON_ID;
        nid.uFlags = NIF_TIP | NIF_ICON | NIF_MESSAGE;
        nid.uCallbackMessage = WM_TRAY;
        nid.hIcon = (HICON)LoadImage(NULL, TEXT(".\\icon.ico"), IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED);
        lstrcpy(nid.szTip, TEXT("Scheduler"));
        Shell_NotifyIcon(NIM_ADD, &nid);
        return 0;
    case WM_COMMAND:
        wmId = LOWORD(wParam);
        wmEvent = HIWORD(wParam);
        switch (wmId){
        case MENU_QUIT_MESSAGE:
            exit(0);
        }
        break;
        return 0;
    case WM_TRAY:
        if (wParam == ICON_ID){
            switch (lParam){
            case WM_RBUTTONUP:
                hPopupMenu = GetSubMenu(hMenubar, 0);
                POINT pt;
                GetCursorPos(&pt);
                SetForegroundWindow(hWnd);
                TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
                SetForegroundWindow(hWnd);
                PostMessage(hWnd, WM_NULL, 0, 0);
                break;
            }
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}
DWORD32 WINAPI scheduler(LPVOID lpParam){
    for (;;){
        time_t t = time(0);
        tm *now = localtime(&t);

        for (int i = 0; i < root["schedule"].size(); i++){
            if (root["schedule"][i]["hour"] == now->tm_hour &&
                root["schedule"][i]["minute"] == now->tm_min){
                if (!ptr[i]){
                    MessageBox(NULL, TEXT("Beep"), TEXT("Beep"), MB_OK);
                    ptr[i] = true;
                }
                else{
                    ptr[i] = false;
                }
            }
        }
        Sleep(30000);
    }
}
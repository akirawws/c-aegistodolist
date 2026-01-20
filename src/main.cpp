#include <windows.h>
#include <windowsx.h>
#include <gdiplus.h>
#include <string>
#include <fstream>
#include <shellapi.h>
#include <cwctype>
#include "core/TaskManager.h"
#include "ui/Renderer.h"

using namespace std;

#pragma comment (lib,"Gdiplus.lib")

TaskManager core;
Renderer ui;
static Screen currentScreen = Screen::Menu;

static const wchar_t* NEW_TASK_FILE = L"new_task.txt";
static bool awaitingTaskFromFile = false;
static FILETIME lastTaskFileWriteTime = { 0,0 };

static wstring Trim(const wstring& s) {
    size_t start = 0;
    while (start < s.size() && iswspace(s[start])) start++;
    size_t end = s.size();
    while (end > start && iswspace(s[end - 1])) end--;
    return s.substr(start, end - start);
}

static bool TryReadWholeFile(const wchar_t* path, wstring& out) {
    wifstream f(path);
    if (!f.is_open()) return false;
    wstring content, line;
    while (getline(f, line)) {
        content += line;
        content += L"\n";
    }
    f.close();
    out = content;
    return true;
}

static void WriteFileText(const wchar_t* path, const wstring& text) {
    wofstream f(path);
    if (!f.is_open()) return;
    f << text;
    f.close();
}

static bool GetFileWriteTime(const wchar_t* path, FILETIME& ft) {
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (!GetFileAttributesExW(path, GetFileExInfoStandard, &data)) return false;
    ft = data.ftLastWriteTime;
    return true;
}

static void StartAddTaskViaTxt(HWND hwnd) {
    WriteFileText(NEW_TASK_FILE, L"");

    FILETIME ft;
    if (GetFileWriteTime(NEW_TASK_FILE, ft)) lastTaskFileWriteTime = ft;

    ShellExecuteW(hwnd, L"open", L"notepad.exe", NEW_TASK_FILE, NULL, SW_SHOWNORMAL);

    awaitingTaskFromFile = true;
    SetTimer(hwnd, 1, 500, NULL);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc; GetClientRect(hwnd, &rc);
        
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBM = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
        SelectObject(memDC, memBM);

        ui.DrawUI(memDC, rc.right, rc.bottom, core.GetTasks(), currentScreen);

        BitBlt(hdc, 0, 0, rc.right, rc.bottom, memDC, 0, 0, SRCCOPY);
        DeleteObject(memBM); DeleteDC(memDC);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_LBUTTONDOWN: {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        if (ui.buttons.empty()) break;
        
        for (int i = ui.buttons.size() - 1; i >= 0; i--) {
            const auto& btn = ui.buttons[i];
            
            if (btn.rect.Contains((REAL)x, (REAL)y)) {
                
                if (btn.name == L"MENU_ADD") {
                    StartAddTaskViaTxt(hwnd);
                }
                else if (btn.name == L"MENU_TASKS") {
                    currentScreen = Screen::Tasks;
                }
                else if (btn.name == L"BACK") {
                    currentScreen = Screen::Menu;
                }
                else if (btn.name == L"ADD") {
                    StartAddTaskViaTxt(hwnd);
                }
                else if (btn.name == L"DEL") {
                    core.DeleteTask(btn.id);
                }
                else if (btn.name == L"TOGGLE") {
                    core.ToggleTask(btn.id);
                }

                InvalidateRect(hwnd, NULL, FALSE);
                return 0; // Обработали клик, выходим
            }
        }
        return 0;
    }

    case WM_TIMER: {
        if (wParam != 1) break;
        if (!awaitingTaskFromFile) break;

        FILETIME ft;
        if (!GetFileWriteTime(NEW_TASK_FILE, ft)) break;

        if (CompareFileTime(&ft, &lastTaskFileWriteTime) == 0) break;

        lastTaskFileWriteTime = ft;

        wstring content;
        if (!TryReadWholeFile(NEW_TASK_FILE, content)) break;

        wstring trimmed = Trim(content);
        if (!trimmed.empty()) {
            size_t nl = trimmed.find(L'\n');
            wstring firstLine = (nl == wstring::npos) ? trimmed : Trim(trimmed.substr(0, nl));
            if (!firstLine.empty()) {
                core.AddTask(firstLine);
            }
            WriteFileText(NEW_TASK_FILE, L"");
            awaitingTaskFromFile = false;
            KillTimer(hwnd, 1);
            currentScreen = Screen::Tasks;
            InvalidateRect(hwnd, NULL, FALSE);
        }
        return 0;
    }

    case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    Gdiplus::GdiplusStartupInput gsi;
    ULONG_PTR token;
    Gdiplus::GdiplusStartup(&token, &gsi, NULL);

    WNDCLASSW wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"TodoJSONClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"Todo JSON Manager",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 450, 700,
        NULL, NULL, hInstance, NULL);

    if (!hwnd) return 0;
    ShowWindow(hwnd, nCmdShow);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    Gdiplus::GdiplusShutdown(token);
    return 0;
}
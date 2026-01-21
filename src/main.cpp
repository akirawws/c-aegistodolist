#include <windows.h>
#include <windowsx.h>
#include <gdiplus.h>
#include <string>
#include <cwctype>
#include "core/TaskManager.h"
#include "ui/Renderer.h"

using namespace std;

#pragma comment (lib,"Gdiplus.lib")

TaskManager core;
Renderer ui;
static Screen currentScreen = Screen::Menu;
static bool showModal = false;
static wstring modalTitle = L"";
static wstring modalDesc = L"";
static bool editingTitle = false;
static int modalYear = 0;
static int modalMonth = 0;
static int modalDay = 0;
static int modalHour = 0;
static int modalMinute = 0;
static COLORREF modalColor = RGB(60, 63, 75);

static wstring FormatDateYMD(int y, int m, int d) {
    wchar_t buf[11];
    wsprintfW(buf, L"%04d-%02d-%02d", y, m, d);
    return buf;
}

static void OpenModal() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    showModal = true;
    modalTitle = L"";
    modalDesc = L"";
    editingTitle = true;
    modalYear = (int)st.wYear;
    modalMonth = (int)st.wMonth;
    modalDay = (int)st.wDay;
    modalHour = (int)st.wHour;      
    modalMinute = (int)st.wMinute;  
    modalColor = RGB(60, 63, 75); 
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
        if (showModal) {
            ui.DrawModal(memDC, rc.right, rc.bottom, modalTitle, modalDesc, editingTitle, modalYear, modalMonth, modalDay, modalHour, modalMinute, modalColor);
        }

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
                if (showModal) {
                    if (btn.name == L"MODAL_TITLE") {
                        editingTitle = true;
                    }
                    else if (btn.name == L"MODAL_DESC") {
                        editingTitle = false;
                    }
                    else if (btn.name == L"MODAL_CANCEL") {
                        showModal = false;
                        modalTitle = L"";
                        modalDesc = L"";
                    }
                    else if (btn.name == L"COLOR") {
                    static const COLORREF predefinedColors[] = {
                        RGB(60, 63, 75), RGB(88, 101, 242), RGB(76, 175, 80),
                        RGB(244, 67, 54), RGB(255, 152, 0), RGB(156, 39, 176),
                        RGB(0, 188, 212), RGB(121, 85, 72)
                    };
                    if (btn.id >= 0 && btn.id < 8) {
                        modalColor = predefinedColors[btn.id];
                    }
                }

                    else if (btn.name == L"MODAL_SAVE") {
                        if (!modalTitle.empty()) {
                            wchar_t timeBuf[10];
                            swprintf_s(timeBuf, L"%02d:%02d", modalHour, modalMinute);
                            wstring timeStr = timeBuf;

                            core.AddTask(modalTitle, modalDesc, FormatDateYMD(modalYear, modalMonth, modalDay), timeStr, modalColor); 
                            showModal = false;
                            modalTitle = L"";
                            modalDesc = L"";
                            if (currentScreen == Screen::Menu) {
                                currentScreen = Screen::Tasks;
                            }
                        }
                    }
                    else if (btn.name == L"TIME_HOUR_UP") {
                        modalHour = (modalHour + 1) % 24;
                    }
                    else if (btn.name == L"TIME_HOUR_DOWN") {
                        modalHour = (modalHour - 1 + 24) % 24;
                    }
                    else if (btn.name == L"TIME_MIN_UP") {
                        modalMinute = (modalMinute + 1) % 60;
                    }
                    else if (btn.name == L"TIME_MIN_DOWN") {
                        modalMinute = (modalMinute - 1 + 60) % 60;
                    }

                    else if (btn.name == L"CAL_PREV") {
                        modalMonth--;
                        if (modalMonth < 1) { modalMonth = 12; modalYear--; }
                        if (modalDay > 28) modalDay = 1;
                    }
                    else if (btn.name == L"CAL_NEXT") {
                        modalMonth++;
                        if (modalMonth > 12) { modalMonth = 1; modalYear++; }
                        if (modalDay > 28) modalDay = 1;
                    }
                    else if (btn.name == L"CAL_DAY") {
                        if (btn.id > 0) modalDay = btn.id;
                    }
                }
                else {
                    if (btn.name == L"MENU_ADD") {
                        OpenModal();
                    }
                    else if (btn.name == L"MENU_TASKS") {
                        currentScreen = Screen::Tasks;
                    }
                    else if (btn.name == L"MENU_ABOUT") {
                        currentScreen = Screen::About;
                    }
                    else if (btn.name == L"BACK") {
                        currentScreen = Screen::Menu;
                    }
                    else if (btn.name == L"ADD") {
                        OpenModal();
                    }
                    else if (btn.name == L"DEL") {
                        core.DeleteTask(btn.id);
                    }
                    else if (btn.name == L"TOGGLE") {
                        core.ToggleTask(btn.id);
                    }
                }

                InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
        }
        return 0;
    }

    case WM_CHAR: {
        if (!showModal) break;
        
        WCHAR ch = (WCHAR)wParam;
        if (ch == VK_RETURN || ch == VK_TAB) {
            if (editingTitle) {
                editingTitle = false;
            }
            else if (ch == VK_RETURN && !modalTitle.empty()) {
                wstring timeStr = L""; 
                wchar_t timeBuf[10];
                swprintf_s(timeBuf, L"%02d:%02d", modalHour, modalMinute);
                timeStr = timeBuf;

                core.AddTask(modalTitle, modalDesc, FormatDateYMD(modalYear, modalMonth, modalDay), timeStr);
                showModal = false;
                modalTitle = L"";
                modalDesc = L"";
                if (currentScreen == Screen::Menu) {
                    currentScreen = Screen::Tasks;
                }
            }
        }
        else if (ch == VK_BACK) {
            if (editingTitle && !modalTitle.empty()) {
                modalTitle.pop_back();
            }
            else if (!editingTitle && !modalDesc.empty()) {
                modalDesc.pop_back();
            }
        }
        else if (ch >= 32 && ch != 127) {
            if (editingTitle && modalTitle.length() < 50) {
                modalTitle += ch;
            }
            else if (!editingTitle && modalDesc.length() < 200) {
                modalDesc += ch;
            }
        }
        InvalidateRect(hwnd, NULL, FALSE);
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

    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"AegisTodo",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 500, 800,
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
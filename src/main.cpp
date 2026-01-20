#include <windows.h>
#include <windowsx.h>
#include <gdiplus.h>
#include <string>
#include "core/TaskManager.h"
#include "ui/Renderer.h"

#pragma comment (lib,"Gdiplus.lib")

TaskManager core;
Renderer ui;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc; GetClientRect(hwnd, &rc);
        
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBM = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
        SelectObject(memDC, memBM);

        // Renderer заполняет вектор кнопок при отрисовке
        ui.DrawUI(memDC, rc.right, rc.bottom, core.GetTasks());

        BitBlt(hdc, 0, 0, rc.right, rc.bottom, memDC, 0, 0, SRCCOPY);
        DeleteObject(memBM); DeleteDC(memDC);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_LBUTTONDOWN: {
        int x = GET_X_LPARAM(lParam);
        int y = GET_Y_LPARAM(lParam);
        if (ui.buttons.empty()) break;
        
        // Проверяем, попал ли клик в одну из кнопок
        // Обратный порядок (с конца), чтобы кнопки поверх списка (Delete) имели приоритет
        for (int i = ui.buttons.size() - 1; i >= 0; i--) {
            const auto& btn = ui.buttons[i];
            
            if (btn.rect.Contains((REAL)x, (REAL)y)) {
                
                if (btn.name == L"ADD") {
                    // Генерация задачи (в будущем тут можно открыть окно ввода)
                    core.AddTask(L"Новая задача #" + std::to_wstring(core.GetTasks().size() + 1));
                }
                else if (btn.name == L"DEL") {
                    core.DeleteTask(btn.id);
                }
                else if (btn.name == L"TOGGLE") {
                    // Если кликнули по строке, но НЕ по кнопке удаления (она выше в цикле проверок, но на всякий случай)
                    core.ToggleTask(btn.id);
                }

                InvalidateRect(hwnd, NULL, FALSE);
                return 0; // Обработали клик, выходим
            }
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
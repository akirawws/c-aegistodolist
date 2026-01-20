#include "Renderer.h"

using namespace Gdiplus;

static void DrawButton(Graphics& g, const RectF& rect, const WCHAR* text, const Color& fill, Font* font, Brush* textBrush, vector<ClickableArea>& buttons, const wstring& name, int id) {
    SolidBrush fillBrush(fill);
    g.FillRectangle(&fillBrush, rect);

    StringFormat centerFormat;
    centerFormat.SetAlignment(StringAlignmentCenter);
    centerFormat.SetLineAlignment(StringAlignmentCenter);
    g.DrawString(text, -1, font, rect, &centerFormat, textBrush);

    buttons.push_back({ name, id, rect });
}

void Renderer::DrawUI(HDC hdc, int width, int height, vector<TodoItem>& tasks, Screen screen) {
    Graphics g(hdc);
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    
    buttons.clear();

    SolidBrush bgBrush(COLOR_BG_DARK);
    g.FillRectangle(&bgBrush, 0, 0, width, height);

    SolidBrush textBrush(COLOR_TEXT);
    FontFamily ff(L"Segoe UI");
    Font fontTitle(&ff, 20, FontStyleBold, UnitPixel);
    Font fontNormal(&ff, 14, FontStyleRegular, UnitPixel);
    Font fontBold(&ff, 16, FontStyleBold, UnitPixel);

    StringFormat centerFormat;
    centerFormat.SetAlignment(StringAlignmentCenter);
    centerFormat.SetLineAlignment(StringAlignmentCenter);

    RectF titleRect(16.0f, 16.0f, (REAL)width - 32.0f, 32.0f);
    g.DrawString(L"Todo", -1, &fontTitle, titleRect, &centerFormat, &textBrush);

    if (screen == Screen::Menu) {
        SolidBrush panelBrush(COLOR_PANEL);
        RectF panelRect(16.0f, 60.0f, (REAL)width - 32.0f, (REAL)height - 76.0f);
        g.FillRectangle(&panelBrush, panelRect);

        RectF btn1(40.0f, 120.0f, (REAL)width - 80.0f, 52.0f);
        RectF btn2(40.0f, 190.0f, (REAL)width - 80.0f, 52.0f);

        DrawButton(g, btn1, L"+ Добавить задачу (через TXT)", COLOR_ACCENT, &fontBold, &textBrush, buttons, L"MENU_ADD", -1);
        DrawButton(g, btn2, L"Мои задачи", Color(255, 90, 94, 110), &fontBold, &textBrush, buttons, L"MENU_TASKS", -1);

        return;
    }

    SolidBrush panelBrush(COLOR_PANEL);
    RectF panelRect(16.0f, 60.0f, (REAL)width - 32.0f, (REAL)height - 76.0f);
    g.FillRectangle(&panelBrush, panelRect);

    RectF backRect(20.0f, 60.0f, 90.0f, 34.0f);
    DrawButton(g, backRect, L"< Назад", Color(255, 70, 74, 90), &fontBold, &textBrush, buttons, L"BACK", -1);

    RectF addBtnRect(120.0f, 60.0f, (REAL)width - 140.0f, 34.0f);
    DrawButton(g, addBtnRect, L"+ Добавить", COLOR_ACCENT, &fontBold, &textBrush, buttons, L"ADD", -1);

    int y = 110;
    for (size_t i = 0; i < tasks.size(); ++i) {
        if (y > height - 50) break;

        RectF rowRect(24.0f, (REAL)y, (REAL)width - 88.0f, 40.0f);
        RectF delBtnRect((REAL)width - 52.0f, (REAL)y + 6.0f, 28.0f, 28.0f);

        Color taskColor = tasks[i].isCompleted ? COLOR_COMPLETED : COLOR_ITEM;
        SolidBrush taskBrush(taskColor);
        g.FillRectangle(&taskBrush, rowRect);

        StringFormat leftFormat;
        leftFormat.SetAlignment(StringAlignmentNear);
        leftFormat.SetLineAlignment(StringAlignmentCenter);

        RectF textRect(rowRect.X + 10.0f, rowRect.Y, rowRect.Width - 10.0f, rowRect.Height);
        g.DrawString(tasks[i].text.c_str(), -1, &fontNormal, textRect, &leftFormat, &textBrush);

        SolidBrush delBrush(COLOR_DELETE);
        g.FillRectangle(&delBrush, delBtnRect);
        g.DrawString(L"X", -1, &fontBold, delBtnRect, &centerFormat, &textBrush);

        buttons.push_back({ L"DEL", tasks[i].id, delBtnRect });
        buttons.push_back({ L"TOGGLE", tasks[i].id, rowRect });

        y += 50;
    }
}
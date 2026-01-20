#include "Renderer.h"

using namespace Gdiplus;

static int DaysInMonth(int year, int month) {
    static const int mdays[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
    int d = mdays[month - 1];
    bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    if (month == 2 && leap) d = 29;
    return d;
}

static int WeekdayMon0(int year, int month, int day) {
    SYSTEMTIME st{};
    st.wYear = (WORD)year;
    st.wMonth = (WORD)month;
    st.wDay = (WORD)day;
    FILETIME ft{};
    if (!SystemTimeToFileTime(&st, &ft)) return 0;
    ULARGE_INTEGER ui{};
    ui.LowPart = ft.dwLowDateTime;
    ui.HighPart = ft.dwHighDateTime;
    const unsigned long long days = ui.QuadPart / 864000000000ULL;
    return (int)(days % 7ULL);
}

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
    g.DrawString(L"AegisTodo", -1, &fontTitle, titleRect, &centerFormat, &textBrush);

    if (screen == Screen::Menu) {
        SolidBrush panelBrush(COLOR_PANEL);
        float contentW = min(420.0f, (REAL)width - 32.0f);
        float contentX = (((REAL)width - contentW) / 2.0f);
        RectF panelRect(contentX, 60.0f, contentW, (REAL)height - 76.0f);
        g.FillRectangle(&panelBrush, panelRect);

        float btnX = panelRect.X + 24.0f;
        float btnW = panelRect.Width - 48.0f;
        RectF btn1(btnX, 120.0f, btnW, 52.0f);
        RectF btn2(btnX, 190.0f, btnW, 52.0f);
        RectF btn3(btnX, 260.0f, btnW, 52.0f);

        DrawButton(g, btn1, L"+ Добавить задачу", COLOR_ACCENT, &fontBold, &textBrush, buttons, L"MENU_ADD", -1);
        DrawButton(g, btn2, L"Мои задачи", Color(255, 90, 94, 110), &fontBold, &textBrush, buttons, L"MENU_TASKS", -1);
        DrawButton(g, btn3, L"About", Color(255, 70, 74, 90), &fontBold, &textBrush, buttons, L"MENU_ABOUT", -1);

        return;
    }

    if (screen == Screen::About) {
        SolidBrush panelBrush(COLOR_PANEL);
        float contentW = min(520.0f, (REAL)width - 32.0f);
        float contentX = (((REAL)width - contentW) / 2.0f);
        RectF panelRect(contentX, 60.0f, contentW, (REAL)height - 76.0f);
        g.FillRectangle(&panelBrush, panelRect);

        RectF backRect(panelRect.X + 4.0f, 60.0f, 110.0f, 34.0f);
        DrawButton(g, backRect, L"< Назад", Color(255, 70, 74, 90), &fontBold, &textBrush, buttons, L"BACK", -1);

        StringFormat leftFormat;
        leftFormat.SetAlignment(StringAlignmentNear);
        leftFormat.SetLineAlignment(StringAlignmentNear);

        Font fontH(&ff, 16, FontStyleBold, UnitPixel);
        Font fontP(&ff, 13, FontStyleRegular, UnitPixel);
        SolidBrush mutedBrush(Color(255, 180, 182, 190));

        RectF h(panelRect.X + 16.0f, 110.0f, panelRect.Width - 32.0f, 26.0f);
        g.DrawString(L"About AegisTodo", -1, &fontH, h, &leftFormat, &textBrush);

        RectF p1(panelRect.X + 16.0f, 150.0f, panelRect.Width - 32.0f, 120.0f);
        g.DrawString(L"Кто сделал: sammo, akira, malevolence\n\n"
                     L"Роли:\n"
                     L"- sammo: работа календаря, шифр\n"
                     L"- akira: основная логика программы\n"
                     L"- malevolence: интерфейс\n", -1, &fontP, p1, &leftFormat, &mutedBrush);

        RectF p2(panelRect.X + 16.0f, 290.0f, panelRect.Width - 32.0f, 80.0f);
        g.DrawString(L"Куда обращаться, если будут проблемы (ссылки):\n"
                     L"- [@sammocoboi]\n"
                     L"- [@ak1rrawws]\n", -1, &fontP, p2, &leftFormat, &mutedBrush);

        return;
    }

    SolidBrush panelBrush(COLOR_PANEL);
    float contentW = min(520.0f, (REAL)width - 32.0f);
    float contentX = (((REAL)width - contentW) / 2.0f);
    RectF panelRect(contentX, 60.0f, contentW, (REAL)height - 76.0f);
    g.FillRectangle(&panelBrush, panelRect);

    RectF backRect(panelRect.X + 4.0f, 60.0f, 90.0f, 34.0f);
    DrawButton(g, backRect, L"< Назад", Color(255, 70, 74, 90), &fontBold, &textBrush, buttons, L"BACK", -1);

    RectF addBtnRect(backRect.X + backRect.Width + 10.0f, 60.0f, panelRect.Width - (backRect.Width + 24.0f), 34.0f);
    DrawButton(g, addBtnRect, L"+ Добавить", COLOR_ACCENT, &fontBold, &textBrush, buttons, L"ADD", -1);

    StringFormat leftFormat;
    leftFormat.SetAlignment(StringAlignmentNear);
    leftFormat.SetLineAlignment(StringAlignmentNear);

    Font fontSmall(&ff, 11, FontStyleRegular, UnitPixel);
    SolidBrush descBrush(Color(255, 180, 182, 190));
    
    wstring taskCount = L"Всего задач: " + to_wstring(tasks.size());
    RectF countRect(panelRect.X + 4.0f, 100.0f, 220.0f, 20.0f);
    SolidBrush countBrush(Color(255, 150, 152, 160));
    g.DrawString(taskCount.c_str(), -1, &fontSmall, countRect, &leftFormat, &countBrush);

    int y = 110;
    
    for (size_t i = 0; i < tasks.size(); ++i) {
        if (y > height - 50) break;

        bool hasDesc = !tasks[i].description.empty();
        float rowHeight = hasDesc ? 60.0f : 40.0f;
        RectF rowRect(panelRect.X + 8.0f, (REAL)y, panelRect.Width - 64.0f, rowHeight);
        RectF delBtnRect(panelRect.X + panelRect.Width - 44.0f, (REAL)y + 6.0f, 28.0f, 28.0f);

        Color taskColor = tasks[i].isCompleted ? COLOR_COMPLETED : COLOR_ITEM;
        SolidBrush taskBrush(taskColor);
        g.FillRectangle(&taskBrush, rowRect);

        RectF checkRect(rowRect.X + 8.0f, rowRect.Y + 8.0f, 20.0f, 20.0f);
        if (tasks[i].isCompleted) {
            SolidBrush checkBrush(COLOR_ACCENT);
            g.FillEllipse(&checkBrush, checkRect);
            g.DrawString(L"✓", -1, &fontBold, checkRect, &centerFormat, &textBrush);
        } else {
            Pen checkPen(Color(255, 120, 123, 140), 2.0f);
            g.DrawEllipse(&checkPen, checkRect);
        }

        RectF textRect(rowRect.X + 35.0f, rowRect.Y + 6.0f, rowRect.Width - 45.0f, 20.0f);
        Color textColor = tasks[i].isCompleted ? Color(255, 150, 152, 160) : COLOR_TEXT;
        SolidBrush taskTextBrush(textColor);
        g.DrawString(tasks[i].text.c_str(), -1, &fontNormal, textRect, &leftFormat, &taskTextBrush);

        if (hasDesc) {
            RectF descRect(rowRect.X + 35.0f, rowRect.Y + 28.0f, rowRect.Width - 45.0f, 20.0f);
            g.DrawString(tasks[i].description.c_str(), -1, &fontSmall, descRect, &leftFormat, &descBrush);
        }
        
        if (!tasks[i].noteDate.empty() || !tasks[i].createdAt.empty()) {
            wstring meta = tasks[i].noteDate;
            if (!tasks[i].createdAt.empty()) {
                if (!meta.empty()) meta += L"  •  ";
                meta += tasks[i].createdAt;
            }
            RectF metaRect(rowRect.X + 35.0f, rowRect.Y + (hasDesc ? 46.0f : 26.0f), rowRect.Width - 45.0f, 14.0f);
            SolidBrush metaBrush(Color(255, 140, 142, 150));
            Font fontMeta(&ff, 10, FontStyleRegular, UnitPixel);
            g.DrawString(meta.c_str(), -1, &fontMeta, metaRect, &leftFormat, &metaBrush);
        }

        SolidBrush delBrush(COLOR_DELETE);
        g.FillRectangle(&delBrush, delBtnRect);
        g.DrawString(L"✕", -1, &fontBold, delBtnRect, &centerFormat, &textBrush);

        buttons.push_back({ L"DEL", tasks[i].id, delBtnRect });
        buttons.push_back({ L"TOGGLE", tasks[i].id, rowRect });

        y += (int)rowHeight + 10;
    }
}

void Renderer::DrawModal(HDC hdc, int width, int height, const wstring& titleText, const wstring& descText, bool editingTitle, int year, int month, int day) {
    Graphics g(hdc);
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    
    buttons.clear();

    SolidBrush overlayBrush(Color(180, 0, 0, 0));
    g.FillRectangle(&overlayBrush, 0, 0, width, height);

    float modalWidth = 400.0f;
    float modalHeight = 440.0f;
    float modalX = ((REAL)width - modalWidth) / 2.0f;
    float modalY = ((REAL)height - modalHeight) / 2.0f;
    RectF modalRect(modalX, modalY, modalWidth, modalHeight);

    SolidBrush modalBrush(COLOR_PANEL);
    g.FillRectangle(&modalBrush, modalRect);

    Pen borderPen(Color(255, 100, 104, 120), 2.0f);
    g.DrawRectangle(&borderPen, modalRect);

    SolidBrush textBrush(COLOR_TEXT);
    FontFamily ff(L"Segoe UI");
    Font fontTitle(&ff, 18, FontStyleBold, UnitPixel);
    Font fontNormal(&ff, 13, FontStyleRegular, UnitPixel);
    Font fontBold(&ff, 14, FontStyleBold, UnitPixel);
    Font fontLabel(&ff, 12, FontStyleRegular, UnitPixel);
    SolidBrush labelBrush(Color(255, 200, 202, 210));

    StringFormat leftFormat;
    leftFormat.SetAlignment(StringAlignmentNear);
    leftFormat.SetLineAlignment(StringAlignmentCenter);

    StringFormat centerFormat;
    centerFormat.SetAlignment(StringAlignmentCenter);
    centerFormat.SetLineAlignment(StringAlignmentCenter);

    RectF titleLabelRect(modalX + 20.0f, modalY + 20.0f, 100.0f, 20.0f);
    g.DrawString(L"Название:", -1, &fontLabel, titleLabelRect, &leftFormat, &labelBrush);

    RectF titleInputRect(modalX + 20.0f, modalY + 45.0f, modalWidth - 40.0f, 32.0f);
    SolidBrush inputBrush(Color(255, 50, 52, 62));
    g.FillRectangle(&inputBrush, titleInputRect);
    Pen inputPen(editingTitle ? COLOR_ACCENT : Color(255, 70, 74, 90), editingTitle ? 2.0f : 1.0f);
    g.DrawRectangle(&inputPen, titleInputRect);
    
    RectF titleTextRect(titleInputRect.X + 8.0f, titleInputRect.Y, titleInputRect.Width - 16.0f, titleInputRect.Height);
    if (titleText.empty()) {
        SolidBrush placeholderBrush(Color(255, 120, 123, 140));
        g.DrawString(L"Введите название...", -1, &fontNormal, titleTextRect, &leftFormat, &placeholderBrush);
    } else {
        g.DrawString(titleText.c_str(), -1, &fontNormal, titleTextRect, &leftFormat, &textBrush);
    }
    buttons.push_back({ L"MODAL_TITLE", -1, titleInputRect });

    RectF descLabelRect(modalX + 20.0f, modalY + 90.0f, 100.0f, 20.0f);
    g.DrawString(L"Описание:", -1, &fontLabel, descLabelRect, &leftFormat, &labelBrush);

    RectF descInputRect(modalX + 20.0f, modalY + 115.0f, modalWidth - 40.0f, 60.0f);
    g.FillRectangle(&inputBrush, descInputRect);
    Pen descPen(!editingTitle ? COLOR_ACCENT : Color(255, 70, 74, 90), !editingTitle ? 2.0f : 1.0f);
    g.DrawRectangle(&descPen, descInputRect);
    
    RectF descTextRect(descInputRect.X + 8.0f, descInputRect.Y + 8.0f, descInputRect.Width - 16.0f, descInputRect.Height - 16.0f);
    StringFormat topFormat;
    topFormat.SetAlignment(StringAlignmentNear);
    topFormat.SetLineAlignment(StringAlignmentNear);
    if (descText.empty()) {
        SolidBrush placeholderBrush(Color(255, 120, 123, 140));
        g.DrawString(L"Введите описание\n(необязательно)...", -1, &fontNormal, descTextRect, &topFormat, &placeholderBrush);
    } else {
        g.DrawString(descText.c_str(), -1, &fontNormal, descTextRect, &topFormat, &textBrush);
    }
    buttons.push_back({ L"MODAL_DESC", -1, descInputRect });

    // Calendar
    RectF dateLabelRect(modalX + 20.0f, modalY + 185.0f, 100.0f, 20.0f);
    g.DrawString(L"Дата:", -1, &fontLabel, dateLabelRect, &leftFormat, &labelBrush);
    wchar_t dateBuf[32];
    wsprintfW(dateBuf, L"%04d-%02d-%02d", year, month, day);
    RectF dateValRect(modalX + 70.0f, modalY + 185.0f, 200.0f, 20.0f);
    g.DrawString(dateBuf, -1, &fontLabel, dateValRect, &leftFormat, &textBrush);

    RectF prevRect(modalX + 20.0f, modalY + 210.0f, 34.0f, 26.0f);
    DrawButton(g, prevRect, L"<", Color(255, 70, 74, 90), &fontBold, &textBrush, buttons, L"CAL_PREV", -1);
    RectF nextRect(modalX + modalWidth - 54.0f, modalY + 210.0f, 34.0f, 26.0f);
    DrawButton(g, nextRect, L">", Color(255, 70, 74, 90), &fontBold, &textBrush, buttons, L"CAL_NEXT", -1);

    const wchar_t* monthNames[] = { L"Январь", L"Февраль", L"Март", L"Апрель", L"Май", L"Июнь", L"Июль", L"Август", L"Сентябрь", L"Октябрь", L"Ноябрь", L"Декабрь" };
    wchar_t monthTitle[64];
    wsprintfW(monthTitle, L"%s %d", monthNames[month - 1], year);
    RectF monthTitleRect(modalX + 60.0f, modalY + 210.0f, modalWidth - 120.0f, 26.0f);
    g.DrawString(monthTitle, -1, &fontLabel, monthTitleRect, &centerFormat, &labelBrush);

    const wchar_t* wd[] = { L"Пн", L"Вт", L"Ср", L"Чт", L"Пт", L"Сб", L"Вс" };
    float gridX = modalX + 20.0f;
    float gridY = modalY + 244.0f;
    float cellW = (modalWidth - 40.0f) / 7.0f;
    float cellH = 26.0f;
    Font fontDay(&ff, 11, FontStyleRegular, UnitPixel);
    for (int i = 0; i < 7; i++) {
        RectF r(gridX + cellW * i, gridY, cellW, cellH);
        g.DrawString(wd[i], -1, &fontDay, r, &centerFormat, &labelBrush);
    }

    int firstWd = WeekdayMon0(year, month, 1);
    int dim = DaysInMonth(year, month);
    int cur = 1;
    for (int row = 0; row < 6; row++) {
        for (int col = 0; col < 7; col++) {
            int idx = row * 7 + col;
            RectF cell(gridX + cellW * col, gridY + cellH * (row + 1), cellW, cellH);
            bool inMonth = idx >= firstWd && cur <= dim;
            if (inMonth) {
                bool isSel = (cur == day);
                if (isSel) {
                    SolidBrush selBrush(COLOR_ACCENT);
                    g.FillRectangle(&selBrush, cell);
                }
                wchar_t nbuf[3];
                wsprintfW(nbuf, L"%d", cur);
                SolidBrush dBrush(isSel ? Color(255, 255, 255, 255) : Color(255, 200, 202, 210));
                g.DrawString(nbuf, -1, &fontDay, cell, &centerFormat, &dBrush);
                buttons.push_back({ L"CAL_DAY", cur, cell });
                cur++;
            }
        }
    }

    RectF cancelBtnRect(modalX + 20.0f, modalY + 395.0f, (modalWidth - 50.0f) / 2.0f, 36.0f);
    DrawButton(g, cancelBtnRect, L"Отмена", Color(255, 70, 74, 90), &fontBold, &textBrush, buttons, L"MODAL_CANCEL", -1);

    RectF saveBtnRect(modalX + 30.0f + (modalWidth - 50.0f) / 2.0f, modalY + 395.0f, (modalWidth - 50.0f) / 2.0f, 36.0f);
    DrawButton(g, saveBtnRect, L"Сохранить", COLOR_ACCENT, &fontBold, &textBrush, buttons, L"MODAL_SAVE", -1);
}
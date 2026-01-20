#include "Renderer.h"

void Renderer::DrawUI(HDC hdc, int width, int height, std::vector<TodoItem>& tasks) {
    using namespace Gdiplus;
    Graphics g(hdc);
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    
    buttons.clear();

    // 1. Фон
    SolidBrush bgBrush(COLOR_BG_DARK);
    g.FillRectangle(&bgBrush, 0, 0, width, height);

    SolidBrush textBrush(COLOR_TEXT);
    FontFamily ff(L"Segoe UI");
    Font fontNormal(&ff, 14, FontStyleRegular, UnitPixel);
    Font fontBold(&ff, 16, FontStyleBold, UnitPixel);

    // 2. Кнопка "ДОБАВИТЬ"
    RectF addBtnRect(20, 10, (float)width - 40, 40);
    SolidBrush btnBrush(Color(255, 88, 101, 242)); 
    g.FillRectangle(&btnBrush, addBtnRect);
    
    StringFormat format;
    format.SetAlignment(StringAlignmentCenter);
    format.SetLineAlignment(StringAlignmentCenter);
    g.DrawString(L"+ ДОБАВИТЬ НОВУЮ ЗАДАЧУ", -1, &fontBold, addBtnRect, &format, &textBrush);

    buttons.push_back({ L"ADD", -1, addBtnRect });

    // 3. Список задач
    int y = 60;
    
    // Используем индекс вместо итератора для безопасности
    for (size_t i = 0; i < tasks.size(); ++i) {
        if (y > height - 40) break; // Защита от выхода за границы окна

        RectF rowRect(20.0f, (float)y, (float)width - 80, 40.0f);
        RectF delBtnRect((float)width - 55, (float)y + 5, 30.0f, 30.0f);

        // Плашка задачи
        SolidBrush taskBrush(tasks[i].isCompleted ? Color(255, 45, 48, 55) : Color(255, 60, 63, 69));
        g.FillRectangle(&taskBrush, rowRect);

        // Текст
        format.SetAlignment(StringAlignmentNear);
        g.DrawString(tasks[i].text.c_str(), -1, &fontNormal, RectF(rowRect.X + 10, rowRect.Y, rowRect.Width - 10, rowRect.Height), &format, &textBrush);

        // Кнопка удаления
        SolidBrush delBrush(Color(255, 200, 50, 50));
        g.FillRectangle(&delBrush, delBtnRect);
        format.SetAlignment(StringAlignmentCenter);
        g.DrawString(L"X", -1, &fontBold, delBtnRect, &format, &textBrush);

        // ВАЖНО: сначала добавляем кнопку удаления, так как она меньше и находится поверх
        buttons.push_back({ L"DEL", tasks[i].id, delBtnRect });
        buttons.push_back({ L"TOGGLE", tasks[i].id, rowRect });

        y += 50;
    }
}
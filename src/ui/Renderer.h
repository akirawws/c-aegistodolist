#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <string>
#include "../core/TaskManager.h"
#include "Theme.h"

struct ClickableArea {
    std::wstring name;
    int id;
    Gdiplus::RectF rect;
};

class Renderer {
public:
    std::vector<ClickableArea> buttons; 
    void DrawUI(HDC hdc, int width, int height, std::vector<TodoItem>& tasks);
};
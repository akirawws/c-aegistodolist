#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <string>
#include "../core/TaskManager.h"
#include "Theme.h"

using namespace std;

struct ClickableArea {
    wstring name;
    int id;
    Gdiplus::RectF rect;
};

enum class Screen {
    Menu = 0,
    Tasks = 1,
    Modal = 2,
    About = 3
};

class Renderer {
public:
    vector<ClickableArea> buttons; 
    void DrawUI(HDC hdc, int width, int height, vector<TodoItem>& tasks, Screen screen);
    void DrawModal(HDC hdc, int width, int height, const wstring& titleText, const wstring& descText, bool editingTitle, int year, int month, int day);
};
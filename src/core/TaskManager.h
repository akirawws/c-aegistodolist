#pragma once
#include <string>
#include <vector>

using namespace std;

struct TodoItem {
    int id;
    wstring text;
    wstring description;
    wstring noteDate;  
    wstring noteTime; 
    wstring createdAt;
    bool isCompleted;
    COLORREF color;
};

class TaskManager {
private:
    vector<TodoItem> tasks;
    int currentIdCounter;
    const string filename = "tasks.json";

    void Save();
    void Load();

    wstring ExtractJsonValue(const wstring& json, const wstring& key);

public:
    TaskManager();
    void AddTask(const wstring& text, const wstring& description = L"", const wstring& noteDate = L"", const wstring& noteTime = L"", COLORREF color = RGB(60, 63, 75));
    void ToggleTask(int id);
    void DeleteTask(int id);
    vector<TodoItem>& GetTasks();
};
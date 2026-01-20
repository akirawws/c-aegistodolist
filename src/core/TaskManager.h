#pragma once
#include <string>
#include <vector>

struct TodoItem {
    int id;
    std::wstring text;
    bool isCompleted;
};

class TaskManager {
private:
    std::vector<TodoItem> tasks;
    int currentIdCounter;
    const std::string filename = "tasks.json"; // Теперь JSON

    void Save();
    void Load();
    
    // Помощник для парсинга значений из JSON строки
    std::wstring ExtractJsonValue(const std::wstring& json, const std::wstring& key);

public:
    TaskManager();
    void AddTask(const std::wstring& text);
    void ToggleTask(int id);
    void DeleteTask(int id);
    std::vector<TodoItem>& GetTasks();
};
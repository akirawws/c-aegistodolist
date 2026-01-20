#pragma once
#include <string>
#include <vector>

using namespace std;

struct TodoItem {
    int id;
    wstring text;
    bool isCompleted;
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
    void AddTask(const wstring& text);
    void ToggleTask(int id);
    void DeleteTask(int id);
    vector<TodoItem>& GetTasks();
};
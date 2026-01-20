#include "TaskManager.h"
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

TaskManager::TaskManager() : currentIdCounter(0) {
    Load();
}

wstring TaskManager::ExtractJsonValue(const wstring& json, const wstring& key) {
    wstring searchKey = L"\"" + key + L"\":";
    size_t pos = json.find(searchKey);
    if (pos == wstring::npos) return L"";

    pos += searchKey.length();
    while (pos < json.length() && (json[pos] == L' ' || json[pos] == L':')) pos++;

    if (pos < json.length() && json[pos] == L'"') {
        size_t end = json.find(L'"', pos + 1);
        if (end == wstring::npos) return L"";
        return json.substr(pos + 1, end - pos - 1);
    }
    
    size_t end = json.find_first_of(L",}\n\r", pos);
    if (end == wstring::npos) end = json.length();
    return json.substr(pos, end - pos);
}

void TaskManager::Save() {
    wofstream file(filename.c_str());
    if (file.is_open()) {
        file << L"[\n";
        for (size_t i = 0; i < tasks.size(); ++i) {
            file << L"  {\n";
            file << L"    \"id\": " << tasks[i].id << L",\n";
            file << L"    \"text\": \"" << tasks[i].text << L"\",\n";
            file << L"    \"isCompleted\": " << (tasks[i].isCompleted ? L"true" : L"false") << L"\n";
            file << L"  }" << (i < tasks.size() - 1 ? L"," : L"") << L"\n";
        }
        file << L"]";
        file.close();
    }
}

void TaskManager::Load() {
    wifstream file(filename.c_str());
    if (!file.is_open()) return;

    wstringstream buffer;
    buffer << file.rdbuf();
    wstring content = buffer.str();
    file.close();

    tasks.clear();
    size_t pos = 0;
    while ((pos = content.find(L"{", pos)) != wstring::npos) {
        size_t end = content.find(L"}", pos);
        if (end == wstring::npos) break;

        wstring obj = content.substr(pos, end - pos + 1);
        TodoItem t;
        
        wstring idStr = ExtractJsonValue(obj, L"id");
        wstring textStr = ExtractJsonValue(obj, L"text");
        wstring boolStr = ExtractJsonValue(obj, L"isCompleted");

        if (!idStr.empty()) {
            try { t.id = stoi(idStr); } catch(...) { t.id = 0; }
        }
        t.text = textStr;
        t.isCompleted = (boolStr.find(L"true") != std::wstring::npos);

        tasks.push_back(t);
        if (t.id >= currentIdCounter) currentIdCounter = t.id + 1;
        pos = end + 1;
    }
}

void TaskManager::AddTask(const wstring& text) {
    tasks.push_back({ currentIdCounter++, text, false });
    Save();
}

void TaskManager::ToggleTask(int id) {
    for (auto& t : tasks) if (t.id == id) { t.isCompleted = !t.isCompleted; break; }
    Save();
}

void TaskManager::DeleteTask(int id) {
    for (auto it = tasks.begin(); it != tasks.end(); ++it) {
        if (it->id == id) { tasks.erase(it); break; }
    }
    Save();
}

vector<TodoItem>& TaskManager::GetTasks() { return tasks; }
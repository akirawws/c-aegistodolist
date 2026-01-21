#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "TaskManager.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <iomanip>

//хуй
using namespace std;

static string WideToUtf8(const wstring& w) {
    if (w.empty()) return {};
    int bytes = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    string out(bytes, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), out.data(), bytes, nullptr, nullptr);
    return out;
}

static wstring Utf8ToWide(const string& s) {
    if (s.empty()) return {};
    int chars = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    wstring out(chars, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), out.data(), chars);
    return out;
}

static wstring EscapeJsonW(const wstring& in) {
    wstring out;
    out.reserve(in.size() + 8);
    for (wchar_t c : in) {
        switch (c) {
        case L'\\': out += L"\\\\"; break;
        case L'"': out += L"\\\""; break;
        case L'\n': out += L"\\n"; break;
        case L'\r': out += L"\\r"; break;
        case L'\t': out += L"\\t"; break;
        default: out.push_back(c); break;
        }
    }
    return out;
}

static wstring UnescapeJsonW(const wstring& in) {
    wstring out;
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); i++) {
        wchar_t c = in[i];
        if (c == L'\\' && i + 1 < in.size()) {
            wchar_t n = in[i + 1];
            switch (n) {
            case L'\\': out.push_back(L'\\'); i++; continue;
            case L'"': out.push_back(L'"'); i++; continue;
            case L'n': out.push_back(L'\n'); i++; continue;
            case L'r': out.push_back(L'\r'); i++; continue;
            case L't': out.push_back(L'\t'); i++; continue;
            default: break;
            }
        }
        out.push_back(c);
    }
    return out;
}

static wstring NowCreatedAt() {
    std::time_t t = std::time(nullptr);
    std::tm tmv{};
    localtime_s(&tmv, &t);
    wchar_t buf[17];
    wcsftime(buf, sizeof(buf) / sizeof(wchar_t), L"%Y-%m-%d %H:%M", &tmv);
    return buf;
}

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
        return UnescapeJsonW(json.substr(pos + 1, end - pos - 1));
    }
    
    size_t end = json.find_first_of(L",}\n\r", pos);
    if (end == wstring::npos) end = json.length();
    return json.substr(pos, end - pos);
}

void TaskManager::Save() {
    ofstream file(filename.c_str(), ios::binary);
    if (!file.is_open()) return;

    wstringstream ws;
    ws << L"[\n";
    for (size_t i = 0; i < tasks.size(); ++i) {
        ws << L"  {\n";
        ws << L"    \"id\": " << tasks[i].id << L",\n";
        ws << L"    \"text\": \"" << EscapeJsonW(tasks[i].text) << L"\",\n";
        ws << L"    \"description\": \"" << EscapeJsonW(tasks[i].description) << L"\",\n";
        ws << L"    \"noteDate\": \"" << EscapeJsonW(tasks[i].noteDate) << L"\",\n";
        ws << L"    \"createdAt\": \"" << EscapeJsonW(tasks[i].createdAt) << L"\",\n";
        ws << L"    \"noteTime\": \"" << EscapeJsonW(tasks[i].noteTime) << L"\",\n";
        ws << L"    \"isCompleted\": " << (tasks[i].isCompleted ? L"true" : L"false") << L"\n";
        ws << L"  }" << (i < tasks.size() - 1 ? L"," : L"") << L"\n";
        ws << L"    \"color\": \"#" 
        << std::hex << std::uppercase << std::setfill(L'0')
        << std::setw(2) << (tasks[i].color & 0xFF)        
        << std::setw(2) << ((tasks[i].color >> 8) & 0xFF) 
        << std::setw(2) << ((tasks[i].color >> 16) & 0xFF) 
        << L"\",\n";
    }
    ws << L"]";

    string utf8 = WideToUtf8(ws.str());
    file.write(utf8.data(), (std::streamsize)utf8.size());
    file.close();
}

void TaskManager::Load() {
    ifstream file(filename.c_str(), ios::binary);
    if (!file.is_open()) return;

    string bytes((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();

    wstring content = Utf8ToWide(bytes);

    tasks.clear();
    size_t pos = 0;
    while ((pos = content.find(L"{", pos)) != wstring::npos) {
        size_t end = content.find(L"}", pos);
        if (end == wstring::npos) break;

        wstring obj = content.substr(pos, end - pos + 1);
        TodoItem t;
        
        wstring idStr = ExtractJsonValue(obj, L"id");
        wstring textStr = ExtractJsonValue(obj, L"text");
        wstring descStr = ExtractJsonValue(obj, L"description");
        wstring dateStr = ExtractJsonValue(obj, L"noteDate");
        wstring createdStr = ExtractJsonValue(obj, L"createdAt");
        wstring timeStr = ExtractJsonValue(obj, L"noteTime");
        wstring boolStr = ExtractJsonValue(obj, L"isCompleted");
        wstring colorStr = ExtractJsonValue(obj, L"color");
        COLORREF color = RGB(60, 63, 75);
        if (!colorStr.empty() && colorStr.length() == 7 && colorStr[0] == L'#') {
            unsigned int r = 0, g = 0, b = 0;
            swscanf_s(colorStr.c_str() + 1, L"%2x%2x%2x", &r, &g, &b);
            color = RGB((BYTE)r, (BYTE)g, (BYTE)b);
        }


        if (!idStr.empty()) {
            try { t.id = stoi(idStr); } catch(...) { t.id = 0; }
        }
        t.text = textStr;
        t.description = descStr;
        t.noteDate = dateStr;
        t.createdAt = createdStr;
        t.noteTime = timeStr;
        t.color = color;
        t.isCompleted = (boolStr.find(L"true") != std::wstring::npos);

        tasks.push_back(t);
        if (t.id >= currentIdCounter) currentIdCounter = t.id + 1;
        pos = end + 1;
    }
}

void TaskManager::AddTask(const wstring& text, const wstring& description, const wstring& noteDate, const wstring& noteTime, COLORREF color) {
    TodoItem item;
    item.id = currentIdCounter++;
    item.text = text;
    item.description = description;
    item.noteDate = noteDate;
    item.noteTime = noteTime;
    item.createdAt = NowCreatedAt();
    item.isCompleted = false;
    item.color = color; 
    tasks.push_back(item);
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
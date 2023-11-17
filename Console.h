// ConsoleWindow.h
#pragma once

#include <vector>
#include <string>
#include <imgui.h>

class Console
{
public:
    Console();

    void ClearLog();
    void AddLog(const char* log);
    void Draw(const char* title);

private:
    std::vector<std::string> items;
    bool scrollToBottom;
};
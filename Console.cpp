#include "Console.h"

Console::Console()
{
    ClearLog();
}

void Console::ClearLog()
{
    items.clear();
    scrollToBottom = true;
}

void Console::AddLog(const char* log)
{
    items.push_back(log);
    scrollToBottom = true;
}

void Console::Draw(const char* title)
{
    ImGui::Begin(title);
    if (ImGui::Button("Clear"))
        ClearLog();
    ImGui::SameLine();
    bool copy = ImGui::Button("Copy");
    ImGui::SameLine();
    bool scroll = ImGui::Button("Scroll to bottom");

    ImGui::Separator();
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    if (copy)
        ImGui::LogToClipboard();

    for (const auto& item : items)
    {
        ImGui::TextUnformatted(item.c_str());
    }

    if (scrollToBottom)
        ImGui::SetScrollY(ImGui::GetScrollMaxY());

    scrollToBottom = false;
    ImGui::EndChild();
    ImGui::End();
}

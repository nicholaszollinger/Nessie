// ConsoleWindow.cpp
#include "EditorConsole.h"
#include "EditorConsole/EditorConsoleLogTarget.h"

namespace nes
{
    EditorConsole::EditorConsole()
    {
        m_desc.m_name = "Console";
        
        // Create the multithreaded console target.
        auto pTarget = std::make_shared<EditorConsoleLogTargetMT>();
        pTarget->SetConsoleWindow(this);
        pTarget->SetPattern(Logger::kDefaultLogPattern);

        // Add to the default logger, so all normal logs go to the editor console.
        auto pDefaultLogger = LoggerRegistry::Instance().GetDefaultLogger();
        pDefaultLogger->AddTarget(pTarget);
        
        m_pConsoleTarget = pTarget;

        Clear();
    }

    EditorConsole::~EditorConsole()
    {
        Clear();

        // Remove the EditorConsole Target from the Logger.
        auto pDefaultLogger = LoggerRegistry::Instance().GetDefaultLogger();
        if (pDefaultLogger != nullptr)
        {
            auto& targets = pDefaultLogger->GetTargets();
            for (size_t i = 0; i < targets.size(); ++i)
            {
                if (std::dynamic_pointer_cast<EditorConsoleLogTargetMT>(targets[i]) != nullptr)
                {
                    std::swap(targets[i], targets.back());
                    targets.pop_back();
                    break;
                }
            }
        }
        
    }

    void EditorConsole::RenderImGui()
    {
        if (!ImGui::Begin("Console", &m_desc.m_isOpen, m_desc.m_flags))
        {
            ImGui::End();
            return;
        }

        // Main Window
        const bool clear = ImGui::Button("Clear");

        // Auto Scroll Check box
        ImGui::SameLine();
        ImGui::Checkbox("Enable Auto Scroll", &m_autoScrollEnabled);
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip(
                "If enabled, the window will automatically continuing scrolling when at scrolled to the bottom.\n"
                "Scrolling up will stop the auto scroll.\n"
            );
        }
        
        // [TODO]: Copy to Clipboard button.
        //ImGui::SameLine();
        //const bool copy = ImGui::Button("Copy");

        // [TODO]: Filter
        //ImGui::SameLine();
        // Filter.Draw("Filter", -100.f);

        ImGui::Separator();

        if (ImGui::BeginChild("Scrolling", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
        {
            if (clear)
                Clear();

            // [TODO]: Copy to clipboard.
            //if (copy)
            //ImGui::CopyToClipboard();

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            const char* pBuffer = m_buffer.begin();
            const char* pBufferEnd = m_buffer.end();

            // The simplest and easy way to display the entire buffer:
            //    ImGui::TextUnformatted(pBuffer, pBufferEnd);
            // And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
            // to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
            // within the visible area.
            // If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
            // on your side is recommended. Using ImGuiListClipper requires
            // - A) random access into your data
            // - B) items all being the  same height,
            // both of which we can handle since we have an array pointing to the beginning of each line of text.
            // When using the filter (in the block of code above) we don't have random access into the data to display
            // anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
            // it possible (and would be recommended if you want to search through tens of thousands of entries).
            ImGuiListClipper clipper;
            clipper.Begin(m_lineOffsets.size());
            while (clipper.Step())
            {
                for (int lineNo = clipper.DisplayStart; lineNo < clipper.DisplayEnd; ++lineNo)
                {
                    const char* lineStart = pBuffer + m_lineOffsets[lineNo];
                    const char* lineEnd = (lineNo + 1 < m_lineOffsets.Size)? (pBuffer + m_lineOffsets[lineNo + 1] - 1) : pBufferEnd;
                    ImGui::TextUnformatted(lineStart, lineEnd);
                }
            }
            clipper.End();
            
            ImGui::PopStyleVar();

            // Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
            // Using a scrollbar or mouse-wheel will take away from the bottom edge.
            if (m_autoScrollEnabled && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);
        }

        ImGui::EndChild();
        ImGui::End();
    }

    void EditorConsole::PostToConsole(const LogMemoryBuffer& message)
    {
        int oldSize = m_buffer.size();
        m_buffer.append(message.begin(), message.end());
        for (int newSize = m_buffer.size(); oldSize < newSize; ++oldSize)
        {
            if (m_buffer[oldSize] == '\n')
                m_lineOffsets.push_back(oldSize + 1);
        }
    }

    void EditorConsole::Clear()
    {
        m_buffer.clear();
        m_lineOffsets.clear();
        m_lineOffsets.push_back(0);
    }
}

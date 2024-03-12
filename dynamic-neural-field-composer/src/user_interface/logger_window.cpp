// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com


#include "user_interface/logger_window.h"


namespace dnf_composer
{
	namespace user_interface
	{
		LoggerWindow::LoggerWindow()
		{
			autoScroll = true;
            isWindowActive = true;
			clean();
		}

        bool LoggerWindow::isActive()
		{
			return isWindowActive;
		}

		void LoggerWindow::render()
		{
            draw();
		}

		void LoggerWindow::clean()
		{
			buffer.clear();
			lineOffsets.clear();
            lineOffsets.push_back(0);
		}

        void LoggerWindow::getLogColor(const char* line_start)
        {
            const char* debugTag = "[DEBUG]";
            const char* infoTag = "[INFO]";
            const char* warningTag = "[WARNING]";
            const char* errorTag = "[ERROR]";
            const char* fatalTag = "[FATAL]";

            // Check if the line contains the DEBUG tag
            if (std::strncmp(line_start, debugTag, std::strlen(debugTag)) == 0)
                textColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);  // Green

            // Check if the line contains the INFO tag
            if (std::strncmp(line_start, infoTag, std::strlen(infoTag)) == 0)
                textColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);  // White

            // Check if the line contains the WARNING tag
            if (std::strncmp(line_start, warningTag, std::strlen(warningTag)) == 0)
                textColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);  // Yellow

            // Check if the line contains the ERROR tag
            if (std::strncmp(line_start, errorTag, std::strlen(errorTag)) == 0)
                textColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);  // Red

            // Check if the line contains the FATAL tag
            if (std::strncmp(line_start, fatalTag, std::strlen(fatalTag)) == 0)
                textColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);  // Red
        }

        void LoggerWindow::drawLog()
		{

            const char* buf = buffer.begin();
            const char* buf_end = buffer.end();

            if (filter.IsActive())
            {
                // In this example we don't use the clipper when Filter is enabled.
                // This is because we don't have random access to the result of our filter.
                // A real application processing logs with ten of thousands of entries may want to store the result of
                // search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
                for (int line_no = 0; line_no < lineOffsets.Size; line_no++)
                {
                    const char* line_start = buf + lineOffsets[line_no];
                    const char* line_end = (line_no + 1 < lineOffsets.Size) ? (buf + lineOffsets[line_no + 1] - 1) : buf_end;

                    getLogColor(line_start);

                    if (filter.PassFilter(line_start, line_end))
                    {
                        ImGui::PushStyleColor(ImGuiCol_Text, textColor);
                        ImGui::TextUnformatted(line_start, line_end);
                        ImGui::PopStyleColor();
                    }
                }
            }
            else
            {
                // The simplest and easy way to display the entire buffer:
                //   ImGui::TextUnformatted(buf_begin, buf_end);
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
                clipper.Begin(lineOffsets.Size);
                while (clipper.Step())
                {
                    for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
                    {
                        const char* line_start = buf + lineOffsets[line_no];
                        const char* line_end = (line_no + 1 < lineOffsets.Size) ? (buf + lineOffsets[line_no + 1] - 1) : buf_end;

                        getLogColor(line_start);

                        ImGui::PushStyleColor(ImGuiCol_Text, textColor);
                        ImGui::TextUnformatted(line_start, line_end);
                        ImGui::PopStyleColor();
                    }
                }
                clipper.End();
            }
		}

		void LoggerWindow::draw()
		{
            if (!ImGui::Begin("Log window"))
            {
                ImGui::End();
                return;
            }

            // Options menu
            if (ImGui::BeginPopup("Options"))
            {
                ImGui::Checkbox("Auto-scroll", &autoScroll);
                ImGui::EndPopup();
            }

            // Main window
            if (ImGui::Button("Options"))
                ImGui::OpenPopup("Options");
            ImGui::SameLine();
            const bool clear = ImGui::Button("Clear");
            ImGui::SameLine();
            const bool copy = ImGui::Button("Copy");
            ImGui::SameLine();
            filter.Draw("Filter", -100.0f);

            ImGui::Separator();

            if (ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar))
            {
                if (clear)
                    clean();
                if (copy)
                    ImGui::LogToClipboard();

                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                drawLog();
                ImGui::PopStyleVar();

                // Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
                // Using a scrollbar or mouse-wheel will take away from the bottom edge.
                if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                    ImGui::SetScrollHereY(1.0f);
            }
            ImGui::EndChild();
            ImGui::End();
		}

        void LoggerWindow::addLog(const char* message, ...)
        {
            int old_size = buffer.size();
            va_list args;
            va_start(args, message);

            // Append the log message
            buffer.appendfv(message, args);

            //buffer.append("\n");
            va_end(args);

			// Update lineOffsets
            for (const int new_size = buffer.size(); old_size < new_size; old_size++)
                if (buffer[old_size] == '\n')
                    lineOffsets.push_back(old_size + 1);
        }

	}
}

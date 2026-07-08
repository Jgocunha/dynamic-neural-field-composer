
#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <array>
#include <chrono>
#include <string>
#include <ctime>
#include <filesystem>
#include <sstream>
#include "utils.h"

namespace FileDialog {

	enum class FileDialogType {
		OpenFile,
		SelectFolder
	};
	enum class FileDialogSortOrder {
		Up,
		Down,
		None
	};

	static bool file_dialog_open = false;
	static FileDialogType file_dialog_open_type = FileDialogType::OpenFile;
	static float padding = 10.0;

	// NOLINTNEXTLINE(readability-function-cognitive-complexity) - adapted third-party ImGui dialog; linear immediate-mode layout not worth restructuring
	inline void ShowFileDialog(const bool* open, char* buffer, unsigned int buffer_size,
		FileDialogType type = FileDialogType::OpenFile)
	{
		static int file_dialog_file_select_index = 0;
		static int file_dialog_folder_select_index = 0;
		static std::string file_dialog_current_path = dnf_composer::tools::utils::getResourceRoot() + "/data/";
		static std::string file_dialog_current_file;
		static std::string file_dialog_current_folder;
		static std::array<char, 500> file_dialog_error = {};
		static FileDialogSortOrder file_name_sort_order = FileDialogSortOrder::None;
		static FileDialogSortOrder size_sort_order = FileDialogSortOrder::None;
		static FileDialogSortOrder date_sort_order = FileDialogSortOrder::None;
		static FileDialogSortOrder type_sort_order = FileDialogSortOrder::None;

		static bool initial_path_set = false;

		if (open != nullptr) {
			// Check if there was already something in the buffer. If so, try to use that path (if it exists).
			// If it doesn't exist, just put them into the current path.
			if (!initial_path_set && strlen(buffer) > 0) {
				auto path = std::filesystem::path(buffer);
				if (std::filesystem::is_directory(path)) {
					file_dialog_current_path = buffer;
				}
				else {
					// Check if this is just a file in a real path. If so, use the real path.
					// If that still doesn't work, use the current path.
					if (std::filesystem::exists(path)) {
						// It's a file! Take the path and set it.
						file_dialog_current_path = path.remove_filename().string();
					}
					else {
						// An invalid path was entered
						file_dialog_current_path = std::filesystem::current_path().string();
					}
				}
				initial_path_set = true;
			}

			const float ui = ImGui::GetIO().FontGlobalScale;
			ImGui::SetNextWindowSize(ImVec2(1100.0F * ui, 450.0F * ui));
			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5F, 0.5F));
			ImGui::SetNextWindowBgAlpha(1.0F);
			const char* window_title = (type == FileDialogType::OpenFile ? "Select a file" : "Select a folder");
			ImGui::Begin(window_title, nullptr, ImGuiWindowFlags_NoResize);

			std::vector<std::filesystem::directory_entry> files;
			std::vector<std::filesystem::directory_entry> folders;
			try {
				for (const auto& p : std::filesystem::directory_iterator(file_dialog_current_path)) {
					if (p.is_directory()) {
						folders.push_back(p);
					}
					else {
						files.push_back(p);
					}
				}
			}
			catch (...) // NOLINT(bugprone-empty-catch) - unreadable directories are simply not listed
			{}

			ImGui::Text("%s", file_dialog_current_path.c_str());

			const float dirW    = 200.0F * ui;
			const float availW  = ImGui::GetContentRegionAvail().x;
			const float filesW  = availW - dirW - ImGui::GetStyle().ItemSpacing.x;
			const float btnRowH = ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y + 6.0F;
			const float listH   = ImGui::GetContentRegionAvail().y - btnRowH - ImGui::GetStyle().ItemSpacing.y;

			ImGui::BeginChild("Directories##1", ImVec2(dirW, listH), 1,
				ImGuiWindowFlags_HorizontalScrollbar);

			if (ImGui::Selectable("..", false, ImGuiSelectableFlags_AllowDoubleClick,
				ImVec2(ImGui::GetContentRegionAvail().x, 0)))
			{
				if (ImGui::IsMouseDoubleClicked(0)) {
					file_dialog_current_path = std::filesystem::path(file_dialog_current_path).parent_path().string();
				}
			}
			for (int i = 0; i < folders.size(); ++i) {
				if (ImGui::Selectable(folders[i].path().stem().string().c_str(),
					i == file_dialog_folder_select_index, ImGuiSelectableFlags_AllowDoubleClick,
					ImVec2(ImGui::GetContentRegionAvail().x, 0)))
				{
					file_dialog_current_file = "";
					if (ImGui::IsMouseDoubleClicked(0)) {
						file_dialog_current_path = folders[i].path().string();
						file_dialog_folder_select_index = 0;
						file_dialog_file_select_index = 0;
						ImGui::SetScrollHereY(0.0F);
						//file_dialog_current_folder = "";
					}
					else {
						file_dialog_folder_select_index = i;
						file_dialog_current_folder = folders[i].path().stem().string();
					}
				}
			}
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::BeginChild("Files##1", ImVec2(filesW, listH), 1,
				ImGuiWindowFlags_HorizontalScrollbar);
			ImGui::Columns(4);
			static float initial_spacing_column_0 = 500.0F;
			if (initial_spacing_column_0 > 0) {
				ImGui::SetColumnWidth(0, initial_spacing_column_0);
				initial_spacing_column_0 = 0.0F;
			}
			static float initial_spacing_column_1 = 100.0F;
			if (initial_spacing_column_1 > 0) {
				ImGui::SetColumnWidth(1, initial_spacing_column_1);
				initial_spacing_column_1 = 0.0F;
			}
			static float initial_spacing_column_2 = 100.0F;
			if (initial_spacing_column_2 > 0) {
				ImGui::SetColumnWidth(2, initial_spacing_column_2);
				initial_spacing_column_2 = 0.0F;
			}
			if (ImGui::Selectable("File")) {
				size_sort_order = FileDialogSortOrder::None;
				date_sort_order = FileDialogSortOrder::None;
				type_sort_order = FileDialogSortOrder::None;
				file_name_sort_order =
					(file_name_sort_order == FileDialogSortOrder::Down ? FileDialogSortOrder::Up : FileDialogSortOrder::Down);
			}
			ImGui::NextColumn();
			if (ImGui::Selectable("Size")) {
				file_name_sort_order = FileDialogSortOrder::None;
				date_sort_order = FileDialogSortOrder::None;
				type_sort_order = FileDialogSortOrder::None;
				size_sort_order =
					(size_sort_order == FileDialogSortOrder::Down ? FileDialogSortOrder::Up : FileDialogSortOrder::Down);
			}
			ImGui::NextColumn();
			if (ImGui::Selectable("Type")) {
				file_name_sort_order = FileDialogSortOrder::None;
				date_sort_order = FileDialogSortOrder::None;
				size_sort_order = FileDialogSortOrder::None;
				type_sort_order =
					(type_sort_order == FileDialogSortOrder::Down ? FileDialogSortOrder::Up : FileDialogSortOrder::Down);
			}
			ImGui::NextColumn();
			if (ImGui::Selectable("Date")) {
				file_name_sort_order = FileDialogSortOrder::None;
				size_sort_order = FileDialogSortOrder::None;
				type_sort_order = FileDialogSortOrder::None;
				date_sort_order =
					(date_sort_order == FileDialogSortOrder::Down ? FileDialogSortOrder::Up : FileDialogSortOrder::Down);
			}
			ImGui::NextColumn();
			ImGui::Separator();

			// Sort files
			if (file_name_sort_order != FileDialogSortOrder::None) {
				std::ranges::sort(files,
				                  [](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b)
				                  {
					                  if (file_name_sort_order == FileDialogSortOrder::Down) {
						                  return a.path().filename().string() > b.path().filename().string();
					                  }
					                  
						                  return a.path().filename().string() < b.path().filename().string();
					                 
				                  });
			}
			else if (size_sort_order != FileDialogSortOrder::None) {
				std::ranges::sort(files,
				                  [](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b)
				                  {
					                  if (size_sort_order == FileDialogSortOrder::Down) {
						                  return a.file_size() > b.file_size();
					                  }
					                  
						                  return a.file_size() < b.file_size();
					                 
				                  });
			}
			else if (type_sort_order != FileDialogSortOrder::None) {
				std::ranges::sort(files,
				                  [](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b)
				                  {
					                  if (type_sort_order == FileDialogSortOrder::Down) {
						                  return a.path().extension().string() > b.path().extension().string();
					                  }
					                  
						                  return a.path().extension().string() < b.path().extension().string();
					                 
				                  });
			}
			else if (date_sort_order != FileDialogSortOrder::None) {
				std::ranges::sort(files,
				                  [](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b)
				                  {
					                  if (date_sort_order == FileDialogSortOrder::Down) {
						                  return a.last_write_time() > b.last_write_time();
					                  }
					                  
						                  return a.last_write_time() < b.last_write_time();
					                 
				                  });
			}

			for (int i = 0; i < files.size(); ++i) {
				if (ImGui::Selectable(files[i].path().filename().string().c_str(),
					i == file_dialog_file_select_index, ImGuiSelectableFlags_AllowDoubleClick,
					ImVec2(ImGui::GetContentRegionAvail().x, 0)))
				{
					file_dialog_file_select_index = i;
					file_dialog_current_file = files[i].path().filename().string();
					file_dialog_current_folder = "";
				}
				ImGui::NextColumn();
				ImGui::TextUnformatted(std::to_string(files[i].file_size()).c_str());
				ImGui::NextColumn();
				ImGui::TextUnformatted(files[i].path().extension().string().c_str());
				ImGui::NextColumn();
				auto ftime = files[i].last_write_time();
				auto st =
					std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - decltype(ftime)::clock::now()
						+ std::chrono::system_clock::now());
				std::time_t tt = std::chrono::system_clock::to_time_t(st);

				std::tm mt{};
				if (!dnf_composer::tools::utils::safe_localtime(&tt, &mt)) {
					// Handle error - you might want to throw an exception or use a default,
					// For example:
					throw std::runtime_error("Failed to convert time");
				}
				std::stringstream ss;
				ss << std::put_time(&mt, "%F %R");

				ImGui::TextUnformatted(ss.str().c_str());
				ImGui::NextColumn();
			}
			ImGui::EndChild();

			// Use std::filesystem::path for proper path concatenation
			std::filesystem::path basePath(file_dialog_current_path);
			std::filesystem::path selectedPath;

			if (!file_dialog_current_folder.empty()) {
				selectedPath = basePath / file_dialog_current_folder;
			} else if (!file_dialog_current_file.empty()) {
				selectedPath = basePath / file_dialog_current_file;
			} else {
				selectedPath = basePath;
			}

			ImGui::PushItemWidth(availW);

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6);

			if (ImGui::Button("New folder")) {
				ImGui::OpenPopup("NewFolderPopup");
			}
			ImGui::SameLine();

			static bool disable_delete_button = false;
			disable_delete_button = (file_dialog_current_folder.empty());
			if (disable_delete_button) {
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5F);
			}
			if (ImGui::Button("Delete folder")) {
				ImGui::OpenPopup("DeleteFolderPopup");
			}
			if (disable_delete_button) {
				ImGui::PopStyleVar();
				ImGui::PopItemFlag();
			}

			ImVec2 center(ImGui::GetWindowPos().x + ImGui::GetWindowSize().x * 0.5F, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y * 0.5F);
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5F, 0.5F));
			if (ImGui::BeginPopup("NewFolderPopup", ImGuiWindowFlags_Modal)) {
				ImGui::Text("Enter a name for the new folder");
				static std::array<char, 500> new_folder_name = {};
				static std::array<char, 500> new_folder_error = {};
				ImGui::InputText("##newfolder", new_folder_name.data(), new_folder_name.size());
				if (ImGui::Button("Create##1")) {
					if (strlen(new_folder_name.data()) <= 0) {
						snprintf(new_folder_error.data(), new_folder_error.size(), "%s", "Folder name can't be empty");
					}
					else {
						// Use std::filesystem::path for proper path concatenation
						std::filesystem::path new_folder_path = std::filesystem::path(file_dialog_current_path) / new_folder_name.data();
						std::filesystem::create_directory(new_folder_path);
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel##1")) {
					snprintf(new_folder_name.data(), new_folder_name.size(), "%s", "");
					snprintf(new_folder_error.data(), new_folder_error.size(), "%s", "");
					ImGui::CloseCurrentPopup();
				}
				ImGui::TextColored(ImColor(1.0F, 0.0F, 0.2F, 1.0F), "%s", new_folder_error.data());
				ImGui::EndPopup();
			}

			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5F, 0.5F));
			if (ImGui::BeginPopup("DeleteFolderPopup", ImGuiWindowFlags_Modal)) {
				ImGui::TextColored(ImColor(1.0F, 0.0F, 0.2F, 1.0F), "Are you sure you want to delete this folder?");
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6);
				ImGui::TextUnformatted(file_dialog_current_folder.c_str());
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6);
				if (ImGui::Button("Yes")) {
					// Use std::filesystem::path for proper path concatenation
					std::filesystem::path folder_to_delete = std::filesystem::path(file_dialog_current_path) / file_dialog_current_folder;
					std::filesystem::remove(folder_to_delete);
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("No")) {
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
			const float cancelW  = ImGui::CalcTextSize("Cancel").x + ImGui::GetStyle().FramePadding.x * 2.0F;
			const float chooseW  = ImGui::CalcTextSize("Choose").x + ImGui::GetStyle().FramePadding.x * 2.0F;
			const float btnGap   = ImGui::GetStyle().ItemSpacing.x;
			const float rightPad = ImGui::GetStyle().WindowPadding.x;
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() - rightPad - chooseW - btnGap - cancelW);

			static auto reset_everything = [&]() {
				file_dialog_file_select_index = 0;
				file_dialog_folder_select_index = 0;
				file_dialog_current_file = "";
				snprintf(file_dialog_error.data(), file_dialog_error.size(), "%s", "");
				initial_path_set = false;
				file_dialog_open = false;
				};

			if (ImGui::Button("Cancel")) {
				reset_everything();
			}
			ImGui::SameLine();
			if (ImGui::Button("Choose")) {
				if (type == FileDialogType::SelectFolder) {
					if (file_dialog_current_folder.empty()) {
						snprintf(file_dialog_error.data(), file_dialog_error.size(), "%s", "Error: You must select a folder!");
					}
					else {
						// Use std::filesystem::path for proper path concatenation
						std::filesystem::path folder_path = std::filesystem::path(file_dialog_current_path) / file_dialog_current_folder;
						std::string path = folder_path.string();
						snprintf(buffer, buffer_size, "%s", path.c_str());
						snprintf(file_dialog_error.data(), file_dialog_error.size(), "%s", "");
						reset_everything();
					}
				}
				else if (type == FileDialogType::OpenFile) {
					if (file_dialog_current_file.empty()) {
						snprintf(file_dialog_error.data(), file_dialog_error.size(), "%s", "Error: You must select a file!");
					}
					else {
						// Use std::filesystem::path for proper path concatenation
						std::filesystem::path file_path = std::filesystem::path(file_dialog_current_path) / file_dialog_current_file;
						std::string path = file_path.string();
						snprintf(buffer, buffer_size, "%s", path.c_str());
						snprintf(file_dialog_error.data(), file_dialog_error.size(), "%s", "");
						reset_everything();
					}
				}
			}

			if (strlen(file_dialog_error.data()) > 0) {
				ImGui::TextColored(ImColor(1.0F, 0.0F, 0.2F, 1.0F), "%s", file_dialog_error.data());
			}

			ImGui::End();
		}
	}

	inline void ShowFileDialog_s(const bool* open, char* buffer, FileDialogType type = FileDialogType::OpenFile)
	{
		ShowFileDialog(open, buffer, 500, type);
	}
}
#include "Gui.hpp"

Gui::Gui()
{
	m_showConsole = false;
	m_showLuaEditor = false;
}

Gui::~Gui()
{
}

void Gui::Menu()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Proxy"))
		{
			if (ImGui::MenuItem("Show proxy", NULL, &m_showConsole));
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Lua"))
		{
			if (ImGui::MenuItem("Lua Editor", NULL, &m_showLuaEditor));
			ImGui::Separator();
			if (ImGui::MenuItem("Lua Proxy API")) {}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}


void Gui::Render()
{
	Menu();
	if (m_showConsole)
		m_proxy.DrawConsole();
	if (m_showLuaEditor)
		m_proxy.DrawLuaEditor();
}

void Gui::Theme()
{
	ImGuiStyle* style = &ImGui::GetStyle();

	style->ChildBorderSize = 1;
	//style->WindowMinSize = ImVec2(700, 400);
	//style->WindowMinSize = ImVec2(imgui_window_width, imgui_window_height);
	style->WindowTitleAlign = ImVec2(0.5, 0.5);
	style->AntiAliasedFill = true;

	style->WindowRounding = 0;
	style->ChildRounding = 0;
	style->FrameRounding = 0;
	style->ScrollbarRounding = 0;
	style->TabRounding = 0;
	style->GrabRounding = 0;
	style->ScrollbarSize = 20.0f;

	style->Colors[ImGuiCol_TitleBg] = ImColor(25, 25, 25, 230);
	style->Colors[ImGuiCol_TitleBgActive] = ImColor(25, 25, 25, 230);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImColor(25, 25, 25, 130);

	style->Colors[ImGuiCol_WindowBg] = ImColor(15, 15, 15, 230);
	style->Colors[ImGuiCol_CheckMark] = ImColor(255, 255, 255, 255);
	style->Colors[ImGuiCol_ChildBg] = ImColor(20, 20, 20, 255);

	style->Colors[ImGuiCol_FrameBg] = ImColor(25, 25, 25, 230);
	style->Colors[ImGuiCol_FrameBgActive] = ImColor(25, 25, 25, 230);
	style->Colors[ImGuiCol_FrameBgHovered] = ImColor(25, 25, 25, 230);

	style->Colors[ImGuiCol_Header] = ImColor(35, 35, 35, 230);
	style->Colors[ImGuiCol_HeaderActive] = ImColor(35, 35, 35, 230);
	style->Colors[ImGuiCol_HeaderHovered] = ImColor(35, 35, 35, 230);

	style->Colors[ImGuiCol_ResizeGrip] = ImColor(35, 35, 35, 255);
	style->Colors[ImGuiCol_ResizeGripActive] = ImColor(35, 35, 35, 255);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImColor(35, 35, 35, 255);

	style->Colors[ImGuiCol_Button] = ImColor(37, 37, 37, 255);
	style->Colors[ImGuiCol_ButtonActive] = ImColor(41, 41, 41, 255);
	style->Colors[ImGuiCol_ButtonHovered] = ImColor(37, 37, 37, 255);

	style->Colors[ImGuiCol_Tab] = ImColor(136, 0, 20, 140);
	style->Colors[ImGuiCol_TabHovered] = ImColor(136, 0, 20, 160);
	style->Colors[ImGuiCol_TabActive] = ImColor(136, 0, 20, 220);

	style->Colors[ImGuiCol_MenuBarBg] = ImColor(100, 0, 20, 90);

	style->Colors[ImGuiCol_SliderGrab] = ImColor(255, 255, 255, 255);
	style->Colors[ImGuiCol_SliderGrabActive] = ImColor(255, 255, 255, 255);
}
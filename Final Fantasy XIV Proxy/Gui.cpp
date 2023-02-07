#include "Gui.hpp"

void ExampleWindow()
{
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    static float f = 0.0f;
    static int counter = 0;

    ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

    ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
    ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

    if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
        counter++;
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();
}

void Gui::Render()
{
	//ExampleWindow();
	m_proxy.DrawConsole();
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

Gui::Gui()
{
}

Gui::~Gui()
{
}

#include "ProxyGui.hpp"


ProxyGui::ProxyGui()
{
	RegisterCommand("help");
	RegisterCommand("history");
	RegisterCommand("clear");
	AddLog({ "", "", "Welcome!", 0, 0 });
	m_logSend = false;
	m_logRecv = false;
	m_copyToClipboard = false;
}

ProxyGui::~ProxyGui()
{
	ClearLog();
}

void ProxyGui::AddLog(LogInput logInput)
{
	logInput.timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	m_history.push_back(logInput);
}

void ProxyGui::ClearLog()
{
	m_history.clear();
}

void ProxyGui::RegisterCommand(const std::string& command)
{
	m_commands.push_back(command);
}

void ProxyGui::ExecuteCommand(std::string command)
{

}

void ProxyGui::Draw()
{
	ImGui::Begin("Proxy", nullptr, ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Copy packets to clipboard")) { m_copyToClipboard = true; }
			if (ImGui::MenuItem("Clear packet log")) { ClearLog(); }
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Nothing to see here!")) {}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
	ImGui::Checkbox("Log Send", &m_logSend);
	ImGui::SameLine(); ImGui::Checkbox("Log Recv", &m_logRecv);
	ImGui::SameLine(); m_filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
	ImGui::Separator();
	ImGui::End();
}

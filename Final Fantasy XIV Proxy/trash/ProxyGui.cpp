#include "ProxyGui.hpp"


ProxyGui::ProxyGui()
{
	RegisterCommand("help", [this](const std::vector<std::string>& args) { Help(); });
	RegisterCommand("history", [this](const std::vector<std::string>& args) { History(); });
	RegisterCommand("clear", [this](const std::vector<std::string>& args) { ClearLog(); });
	RegisterCommand("send", [this](const std::vector<std::string>& args) { g_proxy.SendPacket(args.at(1).c_str()); });
	AddLog({ "", "", "Welcome!", 0, 0 });
	m_logSend = false;
	m_logRecv = false;
	m_copyToClipboard = false;
	m_userInput.resize(16384);
}

ProxyGui::~ProxyGui()
{
	ClearLog();
}

void ProxyGui::AddPacket(Packet packet)
{
	// TODO: implement packet parser and packet identifier
	std::string packetId = "Unknwon";
	std::string opcode = "0x1234";
	char const hexChars[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	std::string packetData;
	for (int i = 0; i < packet.len; i++)
	{
		packetData += hexChars[(packet.buf[i] & 0xF0) >> 4];
		packetData += hexChars[(packet.buf[i] & 0x0F) >> 0];
	}

	AddLog({ packetId, opcode, packetData, packet.len, NULL });
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


void ProxyGui::RegisterCommand(const std::string& command, CommandCallback callback)
{
	m_commands.emplace(command, callback);
}

void ProxyGui::Help()
{
	std::string commands = "";
	for (const auto& registedCommand : m_commands)
	{
		commands += registedCommand.first + "\n";
	}
	AddLog({ "", "", commands, NULL, NULL });
}

void ProxyGui::History()
{
	std::string history = "";
	for (const auto& historyItem : m_commands)
	{
		history += historyItem.first + "\n";
	}
	AddLog({ "", "", history, NULL, NULL });
}

void ProxyGui::ExecuteCommand(std::string command)
{
	m_commandsHistory.push_back(command);
	std::vector<std::string> commandArgs;

	std::stringstream commandStream(command);
	std::string segment;
	while (std::getline(commandStream, segment, ' '))
		commandArgs.push_back(segment);

	if (commandArgs.size() == 0)
	{
		g_proxy.SendPacket(commandArgs.at(1).c_str());
		return;
	}

	auto foundCommand = m_commands.find(commandArgs.at(0));
	if (foundCommand == m_commands.end())
	{
		Help();
		return;
	}
	foundCommand->second(commandArgs);
}

void ProxyGui::Draw()
{
	ImGui::Begin("Proxy", nullptr, ImGuiWindowFlags_MenuBar);
	{
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
				if (ImGui::MenuItem("Nothing to see here yet!")) {}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
		ImGui::Checkbox("Log Send", &m_logSend);
		ImGui::SameLine(); ImGui::Checkbox("Log Recv", &m_logRecv);
		ImGui::SameLine(); m_filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
		ImGui::Separator();

		const float footerHeightReserve = ImGui::GetStyle().ItemSpacing.y + (ImGui::GetFrameHeightWithSpacing() * 2);
		ImGui::BeginChild("PacketRegion", ImVec2(0, -footerHeightReserve), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		{
			//ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
			if (m_copyToClipboard)
				ImGui::LogToClipboard(); // Start logging to clipboard

			if (ImGui::BeginTable("Packets", 5, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInner | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
			{
				ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
				ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None);
				ImGui::TableSetupColumn("Opcode", ImGuiTableColumnFlags_None);
				ImGui::TableSetupColumn("Packet", ImGuiTableColumnFlags_None);
				ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_None);
				ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_None);
				ImGui::TableHeadersRow();

				for (auto logEntry : m_history)
				{
					ImGui::TableNextRow(); // Next row
					ImGui::TableSetColumnIndex(0);
					ImGui::TextWrapped(logEntry.packetId.c_str());
					ImGui::TableSetColumnIndex(1);
					ImGui::TextWrapped(logEntry.opcode.c_str());
					ImGui::TableSetColumnIndex(2);
					ImGui::TextWrapped(logEntry.packetData.c_str());
					ImGui::TableSetColumnIndex(3);
					std::string packetSize = std::to_string(logEntry.sizeOfPacket);
					ImGui::TextWrapped(packetSize.c_str());
					ImGui::TableSetColumnIndex(4);
					std::tm tm;
					std::ostringstream timeStream;
					localtime_s(&tm, &logEntry.timestamp);
					timeStream << std::put_time(&tm, "%H.%M.%S");
					std::string timestamp = timeStream.str();
					ImGui::TextWrapped(timestamp.c_str());
				}
				ImGui::EndTable();
			}
			if (m_copyToClipboard)
				ImGui::LogFinish();
		}
		ImGui::EndChild();
		ImGui::Separator();

		ImGui::Dummy(ImVec2(0.0f, 2.0f));
		ImGui::Columns(2);
		ImGui::SetColumnOffset(1, 60);
		{
			if (ImGui::Button("Send", ImVec2(45, 40))) {};
		}
		ImGui::NextColumn();
		{
			//ImGuiInputTextFlags inputTextFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CtrlEnterForNewLine;
			//if (ImGui::InputTextMultiline("Input", m_userInput.data(), m_userInput.size(), ImVec2(ImGui::GetWindowSize().x / 3, ImGui::GetWindowSize().y / 10), inputTextFlags, NULL, NULL))
			//{
			//	
			//}
			//ImGui::SetItemDefaultFocus();
			//if (reclaim_focus)
			//	ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
		}
	}
	ImGui::End();
}

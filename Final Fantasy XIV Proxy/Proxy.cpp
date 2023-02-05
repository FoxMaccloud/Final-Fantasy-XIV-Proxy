#include "Proxy.hpp"

#define OFFSET_SEND 0x2320
#define OFFSET_RECV 0x11D90

SOCKET g_thisSocketSend;
const char* g_thisBufferSend;
uintptr_t g_thisLenSend;

std::vector<Proxy::LogInput> g_packets;

static void AddLog(Proxy::LogInput logInput)
{
	logInput.timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	g_packets.push_back(logInput);
}

static void AddPacket(Proxy::Packet packet)
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

template<class Func>
static MH_STATUS WINAPI MH_CreateHook(Func* pHookMe, Func* pDetour, Func*& ppOriginal)
{
	return MH_CreateHook(pHookMe, pDetour, (LPVOID*)&ppOriginal);
}

std::add_pointer_t<int PASCAL FAR(SOCKET s, const char* buf, int len, int flags)> oSend;
static int PASCAL FAR hkSend(SOCKET s, const char* buf, int len, int flags)
{
	g_thisSocketSend = s;
	g_thisBufferSend = buf;
	g_thisLenSend = len;

	AddPacket({ s, buf, len, flags });

	return oSend(s, buf, len, flags);
}

Proxy::Proxy()
{
	auto ws2_32info = HelperFunctions::GetModuleInfo(L"WS2_32.dll");
	auto ws2_32send = (decltype(&send))((uintptr_t)ws2_32info.lpBaseOfDll + OFFSET_SEND);
	MH_STATUS minhookSend = MH_CreateHook(ws2_32send, hkSend, oSend);
	MH_EnableHook(ws2_32send);
	
	RegisterCommand("help", [this](const std::vector<std::string>& args) { Help(); });
	RegisterCommand("history", [this](const std::vector<std::string>& args) { History(); });
	RegisterCommand("clear", [this](const std::vector<std::string>& args) { ClearLog(); });
	RegisterCommand("send", [this](const std::vector<std::string>& args) { SendPacket(args.at(1).c_str()); });
	AddLog({ "", "", "Welcome!", 0, 0 });
	m_logSend = false;
	m_logRecv = false;
	m_copyToClipboard = false;
	m_autoScroll = true;
	m_scrollToBottom = false;
	m_userInput.resize(16384, NULL);
}

Proxy::~Proxy()
{
	ClearLog();
}

// TODO: Move "hex to bytes" convertion to it's own function.
void Proxy::SendPacket(const char* packet)
{
	size_t packetLen = strlen(packet);

	if (packetLen < 0) // find size of package structure
		return;

	std::vector<char> sendBuffer(packetLen);
	std::copy(packet, packet + packetLen, sendBuffer.begin());

	size_t i = 0;
	for (size_t count = 0; count < packetLen; ++i, count += 2)
	{
		if (sendBuffer[count] >= 'A')
		{
			sendBuffer[count] -= 'A';
			sendBuffer[count] += 10;
		}
		else
		{
			sendBuffer[count] -= 48;
		}
		if (sendBuffer[count + 1] >= 'A')
		{
			sendBuffer[count + 1] -= 'A';
			sendBuffer[count + 1] += 10;
		}
		else
		{
			sendBuffer[count + 1] -= 48;
		}
		sendBuffer[i] = (__int8)(((char)sendBuffer[count]) * (char)16);
		sendBuffer[i] += (__int8)sendBuffer[count + 1];
	}
	sendBuffer[i] = '\0';

	if (g_thisSocketSend)
	{
		oSend(g_thisSocketSend, sendBuffer.data(), packetLen / 2, NULL);
		return;
	}
	LOG("[!] No socket was found!\n");
}

void Proxy::ClearLog()
{
	g_packets.clear();
}


void Proxy::RegisterCommand(const std::string& command, CommandCallback callback)
{
	m_commands.emplace(command, callback);
}

void Proxy::Help()
{
	std::string commands = "";
	for (const auto& registedCommand : m_commands)
	{
		commands += registedCommand.first + "\n";
	}
	AddLog({ "", "", commands, NULL, NULL });
}

void Proxy::History()
{
	std::string history = "";
	for (const auto& historyItem : m_commandsHistory)
	{
		history += historyItem + "\n";
	}
	AddLog({ "", "", history, NULL, NULL });
}

void Proxy::ExecuteCommand(std::string command)
{
	m_commandsHistory.push_back(command);
	std::vector<std::string> commandArgs;

	std::stringstream commandStream(command);
	std::string segment;
	while (std::getline(commandStream, segment, ' '))
		commandArgs.push_back(segment);

	if (commandArgs.size() == 0)
	{
		SendPacket(commandArgs.at(1).c_str());
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

void Proxy::Draw()
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
		ImGui::SameLine(); ImGui::Checkbox("Auto-scoll", &m_autoScroll);
		ImGui::SameLine(); m_filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
		ImGui::Separator();

		const float footerHeightReserve = ImGui::GetStyle().ItemSpacing.y + (ImGui::GetFrameHeightWithSpacing() * 1.5f);
		ImGui::BeginChild("PacketRegion", ImVec2(0, -footerHeightReserve), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		{
			//ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
			if (m_copyToClipboard)
				ImGui::LogToClipboard(); // Start logging to clipboard

			if (ImGui::BeginTable("Packets", 5, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInner | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
			{
				ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
				// Fix these stupid table weights!
				ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 80);
				ImGui::TableSetupColumn("Opcode", ImGuiTableColumnFlags_WidthFixed, 50);
				ImGui::TableSetupColumn("Data", ImGuiTableColumnFlags_WidthFixed, 200);
				ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 50);
				ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed, 100);
				ImGui::TableHeadersRow();

				for (auto logEntry : g_packets)
				{
					if (!((m_filter.PassFilter(logEntry.opcode.c_str())) ||
						(m_filter.PassFilter(logEntry.packetData.c_str())) ||
						(m_filter.PassFilter(logEntry.packetId.c_str())) ||
						(m_filter.PassFilter(std::to_string(logEntry.sizeOfPacket).c_str()))))
					{
						continue;
					}

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
				if (m_copyToClipboard)
					ImGui::LogFinish();
				if (m_scrollToBottom || (m_autoScroll && (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())))
					ImGui::SetScrollHereY(1.0f);
				m_scrollToBottom = false;
				ImGui::EndTable();
			}
			ImGui::EndChild();
		}
		ImGui::Separator();

		bool reclaimFocusOnInput = false;

		ImGui::Dummy(ImVec2(0.0f, 2.0f));
		ImGui::Columns(2);
		ImGui::SetColumnOffset(1, 60);
		{
			if (ImGui::Button("Send", ImVec2(45, 19)))
			{
				ExecuteCommand(m_userInput.data());
				memset(m_userInput.data(), 0, m_userInput.size());
				reclaimFocusOnInput = true;
			}
		}
		ImGui::NextColumn();
		{
			ImGuiInputTextFlags inputTextFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll;
			if (ImGui::InputText("Input", m_userInput.data(), m_userInput.size(), inputTextFlags, NULL, NULL))
			{
				ExecuteCommand(m_userInput.data());
				memset(m_userInput.data(), 0, m_userInput.size());
				reclaimFocusOnInput = true;
				m_scrollToBottom = true;
			}
			ImGui::SetItemDefaultFocus();
		}
		if (reclaimFocusOnInput)
			ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
	}
	ImGui::End();
}
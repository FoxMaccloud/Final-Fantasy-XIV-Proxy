#pragma once
#include "Proxy.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include <functional>

struct LogInput
{
	std::string packetId;
	std::string opcode;
	std::string packetData;
	int sizeOfPacket;
	time_t timestamp;
};

using CommandCallback = std::function<void(const std::vector<std::string>&)>;

class ProxyGui
{
public:
	ProxyGui();
	~ProxyGui();

	void AddPacket(Packet packet);
	void AddLog(LogInput logInput);
	void ClearLog();
	void Draw();
	void RegisterCommand(const std::string& command, CommandCallback callback);
	void ExecuteCommand(std::string command);


private:
	std::vector<char> m_userInput;
	std::unordered_map<std::string, CommandCallback> m_commands;
	std::vector<std::string> m_commandsHistory;
	uint32_t m_maxLogSize = 2000;
	std::vector<LogInput> m_history;
	ImGuiTextFilter m_filter;

	bool m_logSend;
	bool m_logRecv;

	bool m_copyToClipboard;

	void Help();
	void History();
};
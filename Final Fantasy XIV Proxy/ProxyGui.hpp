#pragma once
#include "Proxy.hpp"
#include <imgui.h>
#include <imgui_internal.h>

struct LogInput
{
	std::string packetId;
	std::string opcode;
	std::string packetData;
	uint32_t sizeOfPacket;
	time_t timestamp;
};

class ProxyGui
{
public:
	ProxyGui();
	~ProxyGui();

	void AddLog(LogInput logInput);
	void ClearLog();
	void Draw();
	void RegisterCommand(const std::string& command);
	void ExecuteCommand(std::string command);


private:
	std::string m_userInput;
	std::vector<std::string> m_commands;
	std::vector<std::string> m_commandsHistory;
	uint32_t m_maxLogSize = 2000;
	std::vector<LogInput> m_history;
	ImGuiTextFilter m_filter;

	bool m_logSend;
	bool m_logRecv;

	bool m_copyToClipboard;
};
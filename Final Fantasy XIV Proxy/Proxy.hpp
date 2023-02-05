#pragma once

#include <iostream>
#include <array>
#include <vector>
#include <Windows.h>
#include <Psapi.h>
#include <chrono>
#include <imgui.h>
#include <imgui_internal.h>
#include <functional>

#include "HelperFunctions.hpp"
#include "Console.hpp"
#include "Hook.hpp"
#include "MinHook.h"

class Proxy
{
	using CommandCallback = std::function<void(const std::vector<std::string>&)>;
public:
	Proxy();
	~Proxy();
	void SendPacket(const char* packet);

	struct Packet
	{
		SOCKET s;
		const char* buf;
		int len;
		int flags;
	};
	struct LogInput
	{
		std::string packetId;
		std::string opcode;
		std::string packetData;
		int sizeOfPacket;
		time_t timestamp;
	};

	void ClearLog();
	void Draw();
	void RegisterCommand(const std::string& command, CommandCallback callback);
	void ExecuteCommand(std::string command);


private:
	std::vector<char> m_userInput;
	std::unordered_map<std::string, CommandCallback> m_commands;
	std::vector<std::string> m_commandsHistory;
	uint32_t m_maxLogSize = 2000;
	ImGuiTextFilter m_filter;

	bool m_logSend;
	bool m_logRecv;

	bool m_copyToClipboard;
	bool m_autoScroll;
	bool m_scrollToBottom;

	void Help();
	void History();
};
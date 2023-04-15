#include "Proxy.hpp"

#define OFFSET_SEND 0x2320
#define OFFSET_RECV 0x11D90

PacketParser g_packetParser;

SOCKET g_thisSocketSend;
const char* g_thisBufferSend;
uintptr_t g_thisLenSend;

bool g_logSend;
bool g_logRecv;

std::optional<std::vector<std::uint8_t>> HexToBytes(std::string hexString);

std::vector<LogInput> g_packets;

std::string g_luaOutput;

static void AddLog(LogInput logInput)
{
	logInput.timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	g_packets.push_back(logInput);
}

static void AddPacket(Proxy::Packet packet)
{
	// TODO: implement packet parser and packet identifier
	
	auto start = (std::uint8_t*)packet.buf;
	auto parsedPacket = g_packetParser.ParsePacket(std::vector<std::uint8_t>(start, start + packet.len));
	
	LOG("Parsed Packet:\n PakcetId: %s\n opcode: %s\n packetData: %s\n size %i\n",
		parsedPacket.packetId,
		parsedPacket.opcode,
		parsedPacket.packetData,
		parsedPacket.sizeOfPacket);

	AddLog(parsedPacket);
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

	if (g_logSend)
	{
		AddPacket({ s, buf, len, flags });
	}

	return oSend(s, buf, len, flags);
}

static int ResizeInputTextCallback(ImGuiInputTextCallbackData* data)
{
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
	{
		auto& vec = *static_cast<std::vector<char>*>(data->UserData);
		vec.resize(static_cast<std::vector<char, std::allocator<char>>::size_type>(data->BufTextLen) + 1);
		data->Buf = vec.data();
	}
	return 0;
}

void Proxy::InitConsole()
{
	auto RegisterCommand = [this](const std::string& command, CommandCallback callback)
	{
		m_commands.emplace(command, callback);
	};

	RegisterCommand("help", [this](const std::vector<std::string>& args) { Help(); });
	RegisterCommand("history", [this](const std::vector<std::string>& args) { History(); });
	RegisterCommand("clear", [this](const std::vector<std::string>& args) { ClearLog(); });
	RegisterCommand("send", [this](const std::vector<std::string>& args) {
	auto bytes = HexToBytes(args.at(1));
	if (bytes)
		SendPacket(bytes.value());});
	AddLog({ "", "", "Welcome!", 0, 0 });
	g_logSend = false;
	g_logRecv = false;
	m_copyToClipboard = false;
	m_autoScroll = true;
	m_scrollToBottom = false;
	m_userInput.resize(1, NULL);
}

void Proxy::InitLuaEditor()
{
	static const char* const keywords[] = {
		"and", "break", "do", "", "else", "elseif", "end", "false", "for", "function", "if", "in", "", "local", "nil", "not", "or", "repeat", "return", "then", "true", "until", "while"
	};
	static const char* const identifiers[] = {
		"assert", "collectgarbage", "dofile", "error", "getmetatable", "ipairs", "loadfile", "load", "loadstring",  "next",  "pairs",  "pcall",  "print",  "rawequal",  "rawlen",  "rawget",  "rawset",
		"select",  "setmetatable",  "tonumber",  "tostring",  "type",  "xpcall",  "_G",  "_VERSION","arshift", "band", "bnot", "bor", "bxor", "btest", "extract", "lrotate", "lshift", "replace",
		"rrotate", "rshift", "create", "resume", "running", "status", "wrap", "yield", "isyieldable", "debug","getuservalue", "gethook", "getinfo", "getlocal", "getregistry", "getmetatable",
		"getupvalue", "upvaluejoin", "upvalueid", "setuservalue", "sethook", "setlocal", "setmetatable", "setupvalue", "traceback", "close", "flush", "input", "lines", "open", "output", "popen",
		"read", "tmpfile", "type", "write", "close", "flush", "lines", "read", "seek", "setvbuf", "write", "__gc", "__tostring", "abs", "acos", "asin", "atan", "ceil", "cos", "deg", "exp", "tointeger",
		"floor", "fmod", "ult", "log", "max", "min", "modf", "rad", "random", "randomseed", "sin", "sqrt", "string", "tan", "type", "atan2", "cosh", "sinh", "tanh",
		"pow", "frexp", "ldexp", "log10", "pi", "huge", "maxinteger", "mininteger", "loadlib", "searchpath", "seeall", "preload", "cpath", "path", "searchers", "loaded", "module", "require", "clock",
		"date", "difftime", "execute", "exit", "getenv", "remove", "rename", "setlocale", "time", "tmpname", "byte", "char", "dump", "find", "format", "gmatch", "gsub", "len", "lower", "match", "rep",
		"reverse", "sub", "upper", "pack", "packsize", "unpack", "concat", "maxn", "insert", "pack", "unpack", "remove", "move", "sort", "offset", "codepoint", "char", "len", "codes", "charpattern",
		"coroutine", "table", "io", "os", "string", "utf8", "bit32", "math", "debug", "package"
	};

	for (auto& k : keywords)
		m_keywords.insert(k);

	for (auto& k : identifiers)
		m_identifiers.insert(k);

	// TODO: make identifiers for comments etc.
	m_textChange = false;

	// Lua Editor
	m_luaState.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::table, sol::lib::jit);

	auto luaPrint = [&](sol::variadic_args args)
	{
		std::string printString = m_luaState["tostring"](args.get<sol::object>(0));
		g_luaOutput += printString + "\n";
	};
	
	auto luaSend = [&](sol::variadic_args args)
	{
		std::string packetArg = m_luaState["tostring"](args.get<sol::object>(0));
		// look for packet in packet lookuptable.
		g_luaOutput += "send has not been implemented yet!\n";
	};
	
	auto luaRecv = [&](sol::variadic_args args)
	{
		std::string packetArg = m_luaState["tostring"](args.get<sol::object>(0));
		// look for packet in packet lookuptable.
		g_luaOutput += "send has not been implemented yet!\n";
	};

	m_luaState.set_function("print", luaPrint);
	m_luaState.set_function("send", luaSend);
	m_luaState.set_function("recv", luaRecv);
}

Proxy::Proxy()
{
	auto ws2_32info = HelperFunctions::GetModuleInfo(L"WS2_32.dll");
	auto ws2_32send = (decltype(&send))((uintptr_t)ws2_32info.lpBaseOfDll + OFFSET_SEND);
	MH_STATUS minhookSend = MH_CreateHook(ws2_32send, hkSend, oSend);
	MH_EnableHook(ws2_32send);
	InitConsole();
	InitLuaEditor();
}

Proxy::~Proxy()
{
	ClearLog();
}


std::optional<std::vector<std::uint8_t>> HexToBytes(std::string hexString)
{
	size_t packetLen = strlen(hexString.c_str());
	if (packetLen <= 0)
	{
		return std::nullopt;
	}

	bool hexPrefix = false;
	if (hexString.compare(0, 2, "0x") == 0)
		hexPrefix = true;
	
	if (hexPrefix)
	{
		hexString.erase(0, 2);
	}
	if (hexString.find_first_not_of("0123456789abcdefABCDEF", 0) != std::string::npos)
	{
		AddLog({ "[ERROR]", "", "Please only hex characters in send\n123ABC", NULL, NULL });
		return std::nullopt;
	}
	if (packetLen % 2 != 0)
	{
		AddLog({ "[ERROR]", "", "Odd packet length", NULL, NULL });
		return std::nullopt;
	}

	std::vector<std::uint8_t> byteArray;

	for (std::size_t i = 0; i < hexString.size(); i += 2)
	{
		std::string hexByte = hexString.substr(i, 2);
		std::uint8_t byte = std::stoi(hexByte, nullptr, 16);
		byteArray.push_back(byte);
	}
	LOG("[!] Sendbuffer %s\n", byteArray.data());
	return byteArray;
}

void Proxy::SendPacket(std::vector<std::uint8_t> packet)
{
	if (packet.size() < 2) // replace with min struct size
	{
		return;
	}

	if (g_thisSocketSend)
	{
		oSend(g_thisSocketSend, (const char*)packet.data(), packet.size(), 0);
		return;
	}
	LOG("[!] No socket was found!\n");
}

void Proxy::ClearLog()
{
	g_packets.clear();
}

void Proxy::Help()
{
	std::string commands = "Commands:\n";
	for (const auto& registedCommand : m_commands)
	{
		commands += " ";
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
	if (command.empty())
		return;

	m_commandsHistory.push_back(command);
	std::vector<std::string> commandArgs;

	std::stringstream commandStream(command);
	std::string segment;
	while (std::getline(commandStream, segment, ' '))
		commandArgs.push_back(segment);
	
	size_t commandArgsSize = commandArgs.size();

	if (std::find(commandArgs.begin(), commandArgs.end(), commandArgs.at(0)) != commandArgs.end())
	{
		if (commandArgsSize == 1)
			AddLog({ commandArgs.at(0), "", "", NULL, NULL});
		if (commandArgsSize == 2)
		{
			std::string data = commandArgs.at(1);
			if (data.compare(0, 2, "0x") == 0)
				AddLog({ commandArgs.at(0), "", data, static_cast<int>(commandArgs.at(1).size()-2), NULL });
			else
				AddLog({ commandArgs.at(0), "", data, static_cast<int>(commandArgs.at(1).size()), NULL });
		}
	}

	if (commandArgsSize == 0)
	{
		auto bytes = HexToBytes(commandArgs.at(1));
		if (bytes)
			SendPacket(bytes.value());
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

void Proxy::DrawConsole()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(700, 400));
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
		ImGui::Checkbox("Log Send", &g_logSend);
		ImGui::SameLine(); ImGui::Checkbox("Log Recv", &g_logRecv);
		ImGui::SameLine(); ImGui::Checkbox("Auto-scoll", &m_autoScroll);
		ImGui::SameLine(); m_filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
		ImGui::Separator();

		const float footerHeightReserve = ImGui::GetStyle().ItemSpacing.y + (ImGui::GetFrameHeightWithSpacing() * 1.5f);
		ImGui::BeginChild("PacketRegion", ImVec2(0, -footerHeightReserve), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		{
			//ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
			if (m_copyToClipboard)
				ImGui::LogToClipboard(); // Start logging to clipboard

			float windowWidth = ImGui::GetWindowContentRegionWidth();
			float nameWidth = (static_cast<float>((700 * 10)) / 100); // 700 -> min window size
			float opcodeWidth = (static_cast<float>((700 * 10)) / 100);
			float sizeWith = (static_cast<float>((700 * 7)) / 100);
			float timeWidth = (static_cast<float>((700 * 18)) / 100);
			float dataWidth = windowWidth - nameWidth - opcodeWidth - sizeWith - timeWidth;
			if (ImGui::BeginTable("Packets", 5, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersInner | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
			{
				ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
				ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, nameWidth);
				ImGui::TableSetupColumn("Opcode", ImGuiTableColumnFlags_WidthFixed, opcodeWidth);
				ImGui::TableSetupColumn("Data", ImGuiTableColumnFlags_WidthFixed, dataWidth);
				ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, sizeWith);
				ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed, timeWidth);
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

					bool color = false;
					if (logEntry.packetId == "[ERROR]")
					{
						color = true;
						ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, 0.4f, 0.4f, 1.0f });
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
					if (color)
					{
						ImGui::PopStyleColor();
					}
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
			ImGuiInputTextFlags inputTextFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll | ImGuiInputTextFlags_CallbackResize;
			if (ImGui::InputText("Input", m_userInput.data(), m_userInput.size(), inputTextFlags, ResizeInputTextCallback, &m_userInput))
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
	ImGui::PopStyleVar();
}

std::string Proxy::ExecuteLua()
{
	std::string output = "";
	try
	{
		std::string luaCode(m_luaEditorData.data());
		sol::function_result result = m_luaState.script(luaCode);

		if (!result.valid())
		{
			sol::error error = result;
			std::string errorMsg = error.what();
			return "Lua error: " + errorMsg;
		}
		output = m_luaState["tostring"](result.get<sol::object>());
	}
	catch (const std::exception& e)
	{
		output = e.what();
	}
	return output;
}

void Proxy::DrawLuaEditor()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(550, 300));
	ImGui::Begin("Lua Editor", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Save"))
			{
				/// save text....
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			ImGui::Text("Nothing to see here yet!");
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View"))
		{
			ImGui::Text("Nothing to see here yet!");
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	int textLines = 1;
	for (auto c : m_luaEditorData)
		if (c == '\n') textLines++;

	std::string headerText = std::to_string(textLines) + " lines";
	ImGui::Text(headerText.c_str());

	ImGui::SameLine(); ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - 52);
	if (ImGui::Button("Run", ImVec2(60, 19)))
	{
		g_luaOutput = "";
		g_luaOutput += "Running script!\n";
		auto returnVal = ExecuteLua();
		g_luaOutput += "\n" + returnVal;
	}

	// Main Text area
	// TODO:
	// Save file
	// Load file
	// Execute file
	// Line numbers
	// Syntax highlighting
	{
		if (m_luaEditorData.empty())
			m_luaEditorData.push_back('\0');

		auto luaEditorFlags = ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CallbackResize;
		ImGui::InputTextMultiline("##LuaEditor", m_luaEditorData.data(), m_luaEditorData.size(), ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetWindowHeight() - (ImGui::GetTextLineHeight() * 16)), luaEditorFlags, ResizeInputTextCallback, &m_luaEditorData);

		ImGui::Separator();
		ImGui::BeginChild("##LuaOutput", ImVec2(0, 0), true);
		ImGui::Text(g_luaOutput.c_str());
		ImGui::EndChild();
	}
	ImGui::PopStyleVar();
	ImGui::End();
}

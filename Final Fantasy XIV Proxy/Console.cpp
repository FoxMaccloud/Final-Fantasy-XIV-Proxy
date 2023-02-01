#include "Console.hpp"

Console::Console()
{
	AllocConsole();
	SetConsoleTitleA("Debug Console");

	freopen_s(reinterpret_cast<FILE**>(stdin), "conin$", "r", stdin);
	freopen_s(reinterpret_cast<FILE**>(stdout), "conout$", "w", stdout);
	freopen_s(reinterpret_cast<FILE**>(stderr), "conerr$", "w", stderr);

	::ShowWindow(GetConsoleWindow(), SW_SHOW);
}

Console::~Console()
{
	fclose(stdin);
	fclose(stdout);
	fclose(stderr);
	FreeConsole();
}
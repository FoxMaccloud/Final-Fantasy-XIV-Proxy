// dllmain.cpp : Defines the entry point for the DLL application.
#include <Windows.h>
#include <iostream>
#include "Console.hpp"
#include "Overlay.hpp"

DWORD64 WINAPI MainThread(HMODULE hModule)
{
	DisableThreadLibraryCalls(hModule);
	{
#if(_DEBUG)
		Console debugConsole;
#endif
		MH_Initialize();
		std::unique_ptr<Overlay> overlay = std::make_unique<Overlay>();
		while (true)
		{
			if (GetAsyncKeyState(VK_DELETE) & 1)
			{
				LOG("[+] Unhooking and Unloading!\n");
				break;
			}
			Sleep(1);
		}
	}
	Sleep(500);
	FreeLibraryAndExitThread(hModule, 0);
	return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, nullptr));
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


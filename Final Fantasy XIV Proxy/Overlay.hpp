#pragma once
#include <Windows.h>
#include <iostream>
#include <cstdio>
#include <mutex>
#include <thread>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#include <d3d11.h>
#include <dxgi1_2.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

#include "Console.hpp"
#include "HelperFunctions.hpp"
#include "MinHook.h"
#include "Hook.hpp"
#include "Gui.hpp"

inline bool g_showMenu = true;

class Overlay
{
public:
	Overlay();
	~Overlay();
private:
};
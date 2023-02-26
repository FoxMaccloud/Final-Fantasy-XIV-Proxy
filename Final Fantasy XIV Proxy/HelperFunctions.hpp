#pragma once

#include "Console.hpp"

#include <Windows.h>
#include <iostream>
#include <Psapi.h>
#include <vector>
#include <string>
#include <cstdint>
#include <utility>
#include <array>
#include <numeric>

#define PROCESS L"ffxiv_dx11.exe"

#define VK_W 0x57
#define	VK_A 0x41
#define VK_S 0x53
#define VK_D 0x44


constexpr float mapCoordScale = 70.0f;
constexpr char hexChars[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

struct vec4
{
	float x, y, z, w;
};
struct vec3
{
	float x, y, z;
};
struct vec2
{
	float x, y;
};

namespace HelperFunctions
{
	MODULEINFO GetModuleInfo(const wchar_t* szModule);
	uintptr_t PatternScan(const wchar_t* module, const char* signature);
	HWND FindMainWindow();
	HWND GetProcessWindow();
	bool WorldToScreen(vec3 pos, vec2& screen, float matrix[16], int m_windowWidth, int m_windowHeight);

	template <typename... Pack>
	__forceinline auto MakeShellcode(Pack&&... args);
};

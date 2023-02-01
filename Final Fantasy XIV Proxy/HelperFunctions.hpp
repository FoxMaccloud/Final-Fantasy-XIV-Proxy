#pragma once

#include <Windows.h>
#include <iostream>
#include <Psapi.h>
#include <vector>
#include <array>
#include <string>

#define PROCESS L"ffxiv_dx11.exe"

#define VK_W 0x57
#define	VK_A 0x41
#define VK_S 0x53
#define VK_D 0x44


constexpr float mapCoordScale = 70.0f;

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
};

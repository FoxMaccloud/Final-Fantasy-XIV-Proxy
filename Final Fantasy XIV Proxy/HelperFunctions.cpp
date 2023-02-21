#include "HelperFunctions.hpp"

template<std::size_t... args>
__forceinline constexpr auto sum() -> std::size_t
{
	return (args + ... + 0);
}

template <typename T, std::size_t... sz>
__forceinline constexpr auto MakeFlattenedArray(std::array<T, sz>... ar)
{
	constexpr std::size_t NB_ARRAY = sizeof...(ar);

	T* datas[NB_ARRAY] = { &ar[0]... };
	constexpr std::size_t lengths[NB_ARRAY] = { ar.size()... };

	constexpr std::size_t FLATLENGTH = sum<ar.size()...>();

	std::array<T, FLATLENGTH> flat_a = { 0 };

	auto index = 0;
	for (auto i = 0; i < NB_ARRAY; i++)
	{
		for (auto j = 0; j < lengths[i]; j++)
		{
			flat_a[index] = datas[i][j];
			index++;
		}
	}

	return flat_a;
}

template <std::size_t N>
__forceinline auto ToShellcodeArray(const char(&first)[N]) -> std::array<std::uint8_t, N - 1>
{
	return *(std::array<uint8_t, N - 1>*)(static_cast<const char*>(first));
}

template <typename T>
__forceinline auto ToShellcodeArray(T first) -> std::array<std::uint8_t, sizeof(T)>
{
	return *(std::array<uint8_t, sizeof(first)>*)(&first);
}

struct handle_data
{
	unsigned long process_id;
	HWND window_handle;
};

//BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lparam)
//{
//	auto& data = *reinterpret_cast<handle_data*>(lparam);
//	unsigned long process_id = 0;
//	GetWindowThreadProcessId(handle, &process_id);
//
//	if (data.process_id != process_id)
//	{
//		return TRUE;
//	}
//	data.window_handle = handle;
//	return FALSE;
//}

static BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam) {
	const auto isMainWindow = [handle]() {
		return GetWindow(handle, GW_OWNER) == nullptr && IsWindowVisible(handle);
	};

	DWORD pID = 0;
	GetWindowThreadProcessId(handle, &pID);

	if (GetCurrentProcessId() != pID || !isMainWindow() || handle == GetConsoleWindow())
		return TRUE;

	*reinterpret_cast<HWND*>(lParam) = handle;

	return FALSE;
}

namespace HelperFunctions {
	MODULEINFO GetModuleInfo(const wchar_t* szModule)
	{
		MODULEINFO modinfo = { 0 };
		HMODULE hModule = GetModuleHandle(szModule);
		if (hModule == 0)
			return modinfo;
		GetModuleInformation(GetCurrentProcess(), hModule, &modinfo, sizeof(MODULEINFO));
		return modinfo;
	}

	// Get's a handle for the process which the dll is injected into.
	HWND FindMainWindow()
	{
		handle_data data{};

		data.process_id = GetCurrentProcessId();
		data.window_handle = nullptr;
		EnumWindows(EnumWindowsCallback, reinterpret_cast<LPARAM>(&data));

		return data.window_handle;
	}

	HWND GetProcessWindow()
	{
		HWND hwnd = nullptr;

		do
		{
			EnumWindows(::EnumWindowsCallback, reinterpret_cast<LPARAM>(&hwnd));
		} while (!hwnd);

		char name[128];
		GetWindowTextA(hwnd, name, RTL_NUMBER_OF(name));

		return hwnd;
	}

	bool WorldToScreen(vec3 pos, vec2& screen, float matrix[16], int m_windowWidth, int m_windowHeight)
	{
		//Matrix-vector Product, multiplying world(eye) coordinates by projection matrix = clipCoords
		vec4 clipCoords;
		clipCoords.x = pos.x * matrix[0] + pos.y * matrix[1] + pos.z * matrix[2] + matrix[3];
		clipCoords.y = pos.x * matrix[4] + pos.y * matrix[5] + pos.z * matrix[6] + matrix[7];
		clipCoords.z = pos.x * matrix[8] + pos.y * matrix[9] + pos.z * matrix[10] + matrix[11];
		clipCoords.w = pos.x * matrix[12] + pos.y * matrix[13] + pos.z * matrix[14] + matrix[15];

		if (clipCoords.w < 120.0f)
			return false;

		//perspective division, dividing by clip.W = Normalized Device Coordinates
		vec3 NDC;
		NDC.x = clipCoords.x / clipCoords.w;
		NDC.y = clipCoords.y / clipCoords.w;
		NDC.z = clipCoords.z / clipCoords.w;

		screen.x = (m_windowWidth / 2 * NDC.x) + (NDC.x + m_windowWidth / 2);
		screen.y = -(m_windowHeight / 2 * NDC.y) + (NDC.y + m_windowHeight / 2);
		return true;
	}

	uintptr_t PatternScan(const wchar_t* module, const char* signature)
	{
		static auto pattern_to_byte = [](const char* pattern) {
			auto bytes = std::vector<char>{};
			auto start = const_cast<char*>(pattern);
			auto end = const_cast<char*>(pattern) + strlen(pattern);

			for (auto current = start; current < end; ++current) {
				if (*current == '?') {
					++current;
					if (*current == '?')
						++current;
					bytes.push_back('\?');
				}
				else {
					bytes.push_back(strtoul(current, &current, 16));
				}
			}
			return bytes;
		};

		MODULEINFO mInfo = GetModuleInfo(module);
		uintptr_t base = (uintptr_t)mInfo.lpBaseOfDll;
		uintptr_t sizeOfImage = (uintptr_t)mInfo.SizeOfImage;
		auto patternBytes = pattern_to_byte(signature);

		uintptr_t patternLength = patternBytes.size();
		auto data = patternBytes.data();

		for (uintptr_t i = 0; i < sizeOfImage - patternLength; i++)
		{
			bool found = true;
			for (uintptr_t j = 0; j < patternLength; j++)
			{
				char a = '\?';
				char b = *(char*)(base + i + j);
				found &= data[j] == a || data[j] == b;
			}
			if (found)
			{
				return base + i;
			}
		}
		return NULL;
	}

	template <typename... Pack>
	__forceinline auto MakeShellcode(Pack&&... args)
	{
		return MakeFlattenedArray(
			ToShellcodeArray(std::forward<Pack>(args))...
		);
	}
}
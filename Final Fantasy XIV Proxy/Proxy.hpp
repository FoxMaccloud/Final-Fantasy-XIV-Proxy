#pragma once
#include <iostream>
#include <array>
#include <vector>
#include <Windows.h>
#include <Psapi.h>
#include <chrono>

#include "HelperFunctions.hpp"
#include "Console.hpp"
#include "Hook.hpp"
#include "MinHook.h"

struct Packet
{
	SOCKET s;
	const char* buf;
	int len;
	int flags;
};

class Proxy
{
public:
	Proxy();
	~Proxy();
	void SendPacket(const char* packet);
private:

};
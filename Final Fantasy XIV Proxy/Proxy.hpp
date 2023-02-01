#pragma once
#include <iostream>
#include <WinSock2.h>
#include <array>
#include <vector>

#include "Console.hpp"
#include "Hook.hpp"
#include "MinHook.h"

class Proxy
{
public:
	Proxy();
	~Proxy();
	void SendPackage(const char* package);
private:

};
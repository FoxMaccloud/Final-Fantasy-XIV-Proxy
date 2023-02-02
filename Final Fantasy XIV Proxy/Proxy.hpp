#pragma once
#include <iostream>
#include <array>
#include <vector>
#include <Windows.h>
#include <Psapi.h>

#include "HelperFunctions.hpp"
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
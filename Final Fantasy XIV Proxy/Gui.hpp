#pragma once
#include <iostream>
#include <imgui.h>

/*
Create proxy
pass ptrs to proxy to different menus
*/

#include "Proxy.hpp"
#include "ProxyGui.hpp"
#include "LuaGui.hpp"

std::unique_ptr<Proxy> proxy = std::make_unique<Proxy>();
std::unique_ptr<ProxyGui> proxyGui = std::make_unique<ProxyGui>(proxy);
std::unique_ptr<LuaGui> proxy = std::make_unique<LuaGui>(proxy);


namespace Gui
{
	void Render();
}
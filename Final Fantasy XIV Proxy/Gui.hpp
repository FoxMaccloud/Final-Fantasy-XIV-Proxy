#pragma once
#include <iostream>
#include <imgui.h>

#include "Proxy.hpp"
#include "ProxyGui.hpp"
#include "LuaGui.hpp"

class Gui
{
public:
	Gui();
	~Gui();
	void Render();

private:
	std::unique_ptr<Proxy> m_proxy;
	ProxyGui m_proxyGui;
	LuaGui m_luaGui;
};
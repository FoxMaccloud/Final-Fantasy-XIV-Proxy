#pragma once
#include <iostream>
#include <imgui.h>

#include "Proxy.hpp"

class Gui
{
public:
	Gui();
	~Gui();
	void Render();
	void Theme();

private:
	Proxy m_proxy;
	bool m_showConsole;
	bool m_showLuaEditor;

	void Menu();
};
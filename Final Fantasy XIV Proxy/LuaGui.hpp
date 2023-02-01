#pragma once
class LuaGui
{
public:
	LuaGui(Proxy& proxy);
	~LuaGui();

private:
	Proxy m_proxy;
};

LuaGui::LuaGui(Proxy& proxy)
{
	m_proxy = proxy;
}

LuaGui::~LuaGui()
{
}
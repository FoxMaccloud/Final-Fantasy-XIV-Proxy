#pragma once
#include "Proxy.hpp"

class ProxyGui
{
public:
	ProxyGui(Proxy& proxy);
	~ProxyGui();

private:
	Proxy m_proxy;
};

ProxyGui::ProxyGui(Proxy& proxy)
{
	m_proxy = proxy;
}

ProxyGui::~ProxyGui()
{
}
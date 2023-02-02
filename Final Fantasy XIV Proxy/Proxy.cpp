#include "Proxy.hpp"

#define OFFSET_SEND 0x2320
#define OFFSET_RECV 0x11D90

SOCKET g_thisSocketSend;
const char* g_thisBufferSend;
uintptr_t g_thisLenSend;

static std::vector<Packet> g_packets;

template<class Func>
static MH_STATUS WINAPI MH_CreateHook(Func* pHookMe, Func* pDetour, Func*& ppOriginal)
{
	return MH_CreateHook(pHookMe, pDetour, (LPVOID*)&ppOriginal);
}

std::add_pointer_t<int PASCAL FAR(SOCKET s, const char* buf, int len, int flags)> oSend;
int PASCAL FAR hkSend(SOCKET s, const char* buf, int len, int flags)
{
	g_thisSocketSend = s;
	g_thisBufferSend = buf;
	g_thisLenSend = len;

	g_packets.push_back({ s, buf, len, flags });

	return oSend(s, buf, len, flags);
}

Proxy::Proxy()
{
	auto ws2_32info = HelperFunctions::GetModuleInfo(L"WS2_32.dll");
	auto ws2_32send = (decltype(&send))((uintptr_t)ws2_32info.lpBaseOfDll + OFFSET_SEND);
	MH_STATUS minhookSend = MH_CreateHook(ws2_32send, hkSend, oSend);
	MH_EnableHook(ws2_32send);
}

Proxy::~Proxy()
{
	g_packets.clear();
	g_packets.~vector();
}

// TODO: Move "hex to bytes" convertion to it's own function.
void Proxy::SendPacket(const char* packet)
{
	size_t packetLen = strlen(packet);

	if (packetLen < 0) // find size of package structure
		return;

	std::vector<char> sendBuffer(packetLen);
	std::copy(packet, packet + packetLen, sendBuffer.begin());

	size_t i = 0;
	for (size_t count = 0; count < packetLen; ++i, count += 2)
	{
		if (sendBuffer[count] >= 'A')
		{
			sendBuffer[count] -= 'A';
			sendBuffer[count] += 10;
		}
		else
		{
			sendBuffer[count] -= 48;
		}
		if (sendBuffer[count + 1] >= 'A')
		{
			sendBuffer[count + 1] -= 'A';
			sendBuffer[count + 1] += 10;
		}
		else
		{
			sendBuffer[count + 1] -= 48;
		}
		sendBuffer[i] = (__int8)(((char)sendBuffer[count]) * (char)16);
		sendBuffer[i] += (__int8)sendBuffer[count + 1];
	}
	sendBuffer[i] = '\0';

	if (g_thisSocketSend)
	{
		oSend(g_thisSocketSend, sendBuffer.data(), packetLen / 2, NULL);
		return;
	}
	LOG("[!] No socket was found!\n");
}


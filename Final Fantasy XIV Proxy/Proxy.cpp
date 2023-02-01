#include "Proxy.hpp"

inline SOCKET g_thisSocketSend;
inline const char* g_thisBufferSend;
inline uintptr_t g_thisLenSend;

std::add_pointer_t<int WSAAPI(SOCKET s, const char* buf, int len, int flags)> oSend;
int WSAAPI hkSend(SOCKET s, const char* buf, int len, int flags)
{
	g_thisSocketSend = s;
	g_thisBufferSend = buf;
	g_thisLenSend = len;

	// Add packet to logs
	LOG("[P] %s\n", buf);

	return oSend(s, buf, len, flags);
}

Proxy::Proxy()
{
	MH_STATUS minhookSend = MH_CreateHook(reinterpret_cast<void**>(&send), &hkSend, reinterpret_cast<void**>(&oSend));
	MH_EnableHook(send);
}

Proxy::~Proxy()
{

}

void Proxy::SendPackage(const char* packet)
{
	int packetLen = strlen(packet);

	if (packetLen < 0) // find size of package structure
		return;

	std::vector<char> sendBuffer(packetLen);
	strncpy(sendBuffer.data(), packet, packetLen);

	size_t i = 0;
	for (size_t count = 0; count < packetLen; ++i, count += 2)
	{
		if (sendBuffer[count] >= 'A') {
			sendBuffer[count] -= 'A';
			sendBuffer[count] += 10;
		}
		else {
			sendBuffer[count] -= 48;
		}
		if (sendBuffer[count + 1] >= 'A') {
			sendBuffer[count + 1] -= 'A';
			sendBuffer[count + 1] += 10;
		}
		else {
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

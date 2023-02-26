#include "PacketParser.hpp"

PacketParser::PacketParser()
{
}

PacketParser::~PacketParser()
{
}

LogInput ParsePacket(std::vector<std::uint8_t> packetData)
{
	if (packetData.size() < sizeof(FFXIVARR_PACKET_HEADER))
	{
		return { "[ERROR]" , "", "Packet parsed is smaller then the PACKET_HEADER!",0, 0 };
	}

	FFXIVARR_PACKET_HEADER* packetHeader = (FFXIVARR_PACKET_HEADER*)packetData.data();
	FFXIVARR_PACKET_SEGMENT_HEADER* segmentHeader = (FFXIVARR_PACKET_SEGMENT_HEADER*)(packetData.data() + sizeof(FFXIVARR_PACKET_HEADER));

	uint16_t packetType = 0;
	switch (segmentHeader->type)
	{
	case SEGMENTTYPE_SESSIONINIT:
		break;
	case SEGMENTTYPE_IPC:
		// Read IPC header to determine packet type
		FFXIVARR_IPC_HEADER* ipc_header = (FFXIVARR_IPC_HEADER*)(packetData.data() + sizeof(FFXIVARR_PACKET_HEADER) + sizeof(FFXIVARR_PACKET_SEGMENT_HEADER));
		packetType = ipc_header->type;
		break;
	case SEGMENTTYPE_KEEPALIVE:
		break;
	case SEGMENTTYPE_ENCRYPTIONINIT:
		break;
	default:
		return { "[ERROR]" , "", "unknown SEGMENT_TYPE!",0, 0 };
		break;
	}

	std::string data = "";
	std::string packetId = "TODO";
	std::string opcode = std::to_string(packetType);

	for (int i = 0; i < packetData.size(); i++)
	{
		data += hexChars[(packetData[i] & 0xF0) >> 4];
		data += hexChars[(packetData[i] & 0x0F) >> 0];
	}

	return { packetId, opcode, NULL, (int)packetData.size(), NULL};
}
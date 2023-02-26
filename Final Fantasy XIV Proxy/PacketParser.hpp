#pragma once
#include <iostream>

#include "HelperFunctions.hpp"


struct LogInput
{
	std::string packetId;
	std::string opcode;
	std::string packetData;
	int sizeOfPacket;
	time_t timestamp;
};

class PacketParser
{
public:
	PacketParser();
	~PacketParser();

	LogInput ParsePacket(std::vector<std::uint8_t> data);

private:

};

// https://xiv.dev/network/packet-structure

struct FFXIVARR_PACKET_HEADER
{
	uint64_t magic[2];
	uint64_t timestamp;
	uint32_t size;
	uint16_t connectionType;
	uint16_t segmentCount;
	uint8_t unknown_20;
	uint8_t isCompressed;
	uint32_t unknown_24;
};

struct FFXIVARR_PACKET_SEGMENT_HEADER
{
	uint32_t size;
	uint32_t source_actor;
	uint32_t target_actor;
	uint16_t type;
	uint16_t padding;
};

enum FFXIVARR_SEGMENT_TYPE
{
	SEGMENTTYPE_SESSIONINIT = 1,
	SEGMENTTYPE_IPC = 3,
	SEGMENTTYPE_KEEPALIVE = 7,
	//SEGMENTTYPE_RESPONSE = 8,
	SEGMENTTYPE_ENCRYPTIONINIT = 9,
};

struct FFXIVARR_IPC_HEADER
{
	uint16_t reserved;
	uint16_t type;
	uint16_t padding;
	uint16_t serverId;
	uint32_t timestamp;
	uint32_t padding1;
};

namespace opcodes
{
	const uint16_t unknown = 0x1234;
};
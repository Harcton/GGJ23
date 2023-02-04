#pragma once

#include "Base/DemoContext.h"

namespace se
{
	namespace net
	{
		template<typename>
		class Packetman;
	}
}

enum class PacketType : uint8_t;


struct ClientContext : public DemoContext
{
	se::net::Packetman<PacketType>& packetman;
	const ClientId myClientId;
};

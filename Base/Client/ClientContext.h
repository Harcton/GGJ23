#pragma once

#include "Base/DemoContext.h"
#include "Base/RootStrain.h"

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
	const RootStrain startingRootStrain;
};

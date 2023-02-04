#pragma once

#include "Base/Net/Packets.h"

namespace se
{
	namespace net
	{
		class Connection2;
	}
}


struct Client
{
	Client(const std::shared_ptr<se::net::Connection2>& _connection)
		: packetman(*_connection)
	{
	}
	ClientId clientId;
	std::string name;
	std::shared_ptr<se::net::Connection2> connection;
	se::net::Packetman<PacketType> packetman;
};

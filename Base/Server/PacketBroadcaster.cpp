#include "stdafx.h"
#include "Base/Server/PacketBroadcaster.h"

#include "Base/Net/Packets.h"
#pragma optimize("", off)

struct PacketBroadcaster::Impl
{
	Impl(ServerContext& _context)
		: context(_context)
	{
		for (std::unique_ptr<Client>& client : context.clients)
		{
			client->packetman.update();
			startBroadcastingToOthers<BulletCreatePacket>(client.get(), PacketType::BulletCreate);
		}
	}

	template<typename Packet>
	void startBroadcastingToOthers(Client* const client, const PacketType packetType)
	{
		client->packetman.registerReceiveHandler<Packet>(packetType, scopedConnections.add(),
			[this, client, packetType](Packet& _packet, const bool _reliable)
			{
				for (std::unique_ptr<Client>& c : context.clients)
				{
					if (c.get() != client)
					{
						c->packetman.sendPacket(packetType, _packet, _reliable);
					}
				}
			});
	}

	ServerContext& context;
	se::ScopedConnections scopedConnections;
	std::unordered_map<ClientId, PlayerUpdatePacket> playerUpdatePackets;
	se::time::Time lastUpdateTime;
};

PacketBroadcaster::PacketBroadcaster(ServerContext& _context)
	: impl(new Impl(_context))
{
}

PacketBroadcaster::~PacketBroadcaster()
{
	// ~Impl()
}

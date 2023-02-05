#include "stdafx.h"
#include "Base/Server/PlayerCharacterServer.h"

#include "Base/Net/Packets.h"
#include "Base/MutationDatabase.h"
#include "SpehsEngine/Net/ConnectionManager2.h"
#pragma optimize("", off)

struct PlayerCharacterServer::Impl
{
	Impl(ServerContext& _context)
		: context(_context)
	{
		for (std::unique_ptr<Client>& c : context.clients)
		{
			Client* const client = c.get();
			client->packetman.update();
			client->packetman.registerReceiveHandler<PlayerUpdatePacket>(PacketType::PlayerUpdate, scopedConnections.add(),
				[this, client](PlayerUpdatePacket& _packet, const bool _reliable)
				{
					//se::log::info("Player update received: " + client->name + ": " + std::to_string(_packet.position.x) + ": " + std::to_string(_packet.position.y));
					playerUpdatePackets[client->clientId] = _packet;
				});
			for (const Mutation* const mutation : context.mutationDatabase.vector)
			{
				if (mutation->rootStrain && *mutation->rootStrain == client->rootStrainLoadout)
				{
					PlayerMutatePacket packet;
					packet.stacks = 1;
					packet.mutationId = mutation->mutationId;
					client->packetman.sendPacket(PacketType::PlayerMutated, packet, true);
				}
			}
			
		}
	}

	void update()
	{
		if (se::time::timeSince(lastUpdateTime) > se::time::fromSeconds(1.0f / 20.0f))
		{
			PlayerUpdatesPacket packet;
			packet.playerUpdatePackets = playerUpdatePackets;
			for (std::unique_ptr<Client>& client : context.clients)
			{
				client->packetman.sendPacket(PacketType::PlayerUpdates, packet, false);
			}
		}
	}

	ServerContext& context;
	se::ScopedConnections scopedConnections;
	std::unordered_map<ClientId, PlayerUpdatePacket> playerUpdatePackets;
	se::time::Time lastUpdateTime;
};

PlayerCharacterServer::PlayerCharacterServer(ServerContext& _context)
	: impl(new Impl(_context))
{
}

PlayerCharacterServer::~PlayerCharacterServer()
{
	// ~Impl()
}

void PlayerCharacterServer::update()
{
	impl->update();
}

const std::unordered_map<ClientId, PlayerUpdatePacket>& PlayerCharacterServer::getPlayerUpdatePackets() const
{
	return impl->playerUpdatePackets;
}

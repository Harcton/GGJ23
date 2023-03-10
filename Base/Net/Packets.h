#pragma once

#include "Base/RootStrain.h"


enum class PacketType : uint8_t
{
	LobbyEnter,
	LobbyReady,
	LobbyStart,
	PlayerUpdate,
	PlayerUpdates,
	PlayerMutated,
	BulletCreate,
	RootCreate,
	RootUpdate,
	RootRemove,
	RootDamage,
	GameEnd,
};

// Client -> server
struct LobbyEnterPacket
{
	void write(se::WriteBuffer& writeBuffer) const
	{
		se_write(writeBuffer, name);
		se_write(writeBuffer, color);
	}
	bool read(se::ReadBuffer& readBuffer)
	{
		se_read(readBuffer, name);
		se_read(readBuffer, color);
		return true;
	}
	std::string name;
	se::Color color;
};

struct LobbyEnterResult
{
	void write(se::WriteBuffer& writeBuffer) const
	{
		se_write(writeBuffer, clientId);
		se_write(writeBuffer, startingRootStrain);
	}
	bool read(se::ReadBuffer& readBuffer)
	{
		se_read(readBuffer, clientId);
		se_read(readBuffer, startingRootStrain);
		return true;
	}
	ClientId clientId;
	RootStrain startingRootStrain;
};

// Client -> server
struct LobbyReadyPacket
{
	void write(se::WriteBuffer& writeBuffer) const
	{
		se_write(writeBuffer, ready);
	}
	bool read(se::ReadBuffer& readBuffer)
	{
		se_read(readBuffer, ready);
		return true;
	}
	bool ready = false;
};

// Server -> client
struct LobbyStartPacket
{
	void write(se::WriteBuffer& writeBuffer) const
	{
		se_write(writeBuffer, data);
		se_write(writeBuffer, clientColors);
	}
	bool read(se::ReadBuffer& readBuffer)
	{
		se_read(readBuffer, data);
		se_read(readBuffer, clientColors);
		return true;
	}
	bool data = false;
	std::unordered_map<ClientId, se::Color> clientColors;
};

// Client <-> server
struct PlayerUpdatePacket
{
	void write(se::WriteBuffer& writeBuffer) const
	{
		se_write(writeBuffer, position);
		se_write(writeBuffer, facing);
		se_write(writeBuffer, rootStrainLoadout);
	}
	bool read(se::ReadBuffer& readBuffer)
	{
		se_read(readBuffer, position);
		se_read(readBuffer, facing);
		se_read(readBuffer, rootStrainLoadout);
		return true;
	}
	glm::vec2 position;
	glm::vec2 facing;
	RootStrain rootStrainLoadout = RootStrain::Pink;
};

// Server -> client
struct PlayerUpdatesPacket
{
	void write(se::WriteBuffer& writeBuffer) const
	{
		se_write(writeBuffer, playerUpdatePackets);
	}
	bool read(se::ReadBuffer& readBuffer)
	{
		se_read(readBuffer, playerUpdatePackets);
		return true;
	}
	std::unordered_map<ClientId, PlayerUpdatePacket> playerUpdatePackets;
};

struct PlayerMutatePacket
{
	void write(se::WriteBuffer& writeBuffer) const
	{
		se_write(writeBuffer, mutationId);
		se_write(writeBuffer, stacks);
	}
	bool read(se::ReadBuffer& readBuffer)
	{
		se_read(readBuffer, mutationId);
		se_read(readBuffer, stacks);
		return true;
	}
	MutationId mutationId;
	uint16_t stacks = 0;
	bool clearPrevious = false;
};

// Client <-> server
struct BulletCreatePacket
{
	void write(se::WriteBuffer& writeBuffer) const
	{
		se_write(writeBuffer, position2D);
		se_write(writeBuffer, direction2D);
		se_write(writeBuffer, range);
		se_write(writeBuffer, speed);
		se_write(writeBuffer, damage);
		se_write(writeBuffer, rootStrain);
	}
	bool read(se::ReadBuffer& readBuffer)
	{
		se_read(readBuffer, position2D);
		se_read(readBuffer, direction2D);
		se_read(readBuffer, range);
		se_read(readBuffer, speed);
		se_read(readBuffer, damage);
		se_read(readBuffer, rootStrain);
		return true;
	}
	glm::vec2 position2D;
	glm::vec2 direction2D;
	float range = 0.0f;
	float speed = 0.0f;
	float damage = 0.0f;
	RootStrain rootStrain = RootStrain::Blue;
};

// Server -> client
struct RootCreatePacket
{
	void write(se::WriteBuffer& writeBuffer) const
	{
		se_write(writeBuffer, start);
		se_write(writeBuffer, end);
		se_write(writeBuffer, rootId);
		se_write(writeBuffer, parentRootId);
		se_write(writeBuffer, health);
		se_write(writeBuffer, rootStrain);
	}
	bool read(se::ReadBuffer& readBuffer)
	{
		se_read(readBuffer, start);
		se_read(readBuffer, end);
		se_read(readBuffer, rootId);
		se_read(readBuffer, parentRootId);
		se_read(readBuffer, health);
		se_read(readBuffer, rootStrain);
		return true;
	}
	glm::vec2 start;
	glm::vec2 end;
	RootId rootId;
	RootId parentRootId;
	float health = 0.0f;
	RootStrain rootStrain = RootStrain::Blue;
};

// Server -> client
struct RootUpdatePacket
{
	void write(se::WriteBuffer& writeBuffer) const
	{
		se_write(writeBuffer, rootId);
		se_write(writeBuffer, health);
	}
	bool read(se::ReadBuffer& readBuffer)
	{
		se_read(readBuffer, rootId);
		se_read(readBuffer, health);
		return true;
	}
	RootId rootId;
	float health = 0.0f;
};

// Server -> client
struct RootRemovePacket
{
	void write(se::WriteBuffer& writeBuffer) const
	{
		se_write(writeBuffer, rootId);
	}
	bool read(se::ReadBuffer& readBuffer)
	{
		se_read(readBuffer, rootId);
		return true;
	}
	RootId rootId;
	float health = 0.0f;
};

// Client -> server
struct RootDamagePacket
{
	void write(se::WriteBuffer& writeBuffer) const
	{
		se_write(writeBuffer, rootId);
		se_write(writeBuffer, damage);
	}
	bool read(se::ReadBuffer& readBuffer)
	{
		se_read(readBuffer, rootId);
		se_read(readBuffer, damage);
		return true;
	}
	RootId rootId;
	float damage = 0.0f;
};

// Server -> client
struct GameEndPacket
{
	void write(se::WriteBuffer& writeBuffer) const
	{
		se_write(writeBuffer, win);
	}
	bool read(se::ReadBuffer& readBuffer)
	{
		se_read(readBuffer, win);
		return true;
	}
	bool win = false;
};

#pragma once


enum class PacketType : uint8_t
{
	LobbyEnter,
	LobbyReady,
	LobbyStart,
	PlayerUpdate,
	PlayerUpdates,
	BulletCreate,
};

// Client -> server
struct LobbyEnterPacket
{
	void write(se::WriteBuffer& writeBuffer) const
	{
		se_write(writeBuffer, name);
	}
	bool read(se::ReadBuffer& readBuffer)
	{
		se_read(readBuffer, name);
		return true;
	}
	std::string name;
};

struct LobbyEnterResult
{
	void write(se::WriteBuffer& writeBuffer) const
	{
		se_write(writeBuffer, clientId);
	}
	bool read(se::ReadBuffer& readBuffer)
	{
		se_read(readBuffer, clientId);
		return true;
	}
	ClientId clientId;
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
	}
	bool read(se::ReadBuffer& readBuffer)
	{
		se_read(readBuffer, data);
		return true;
	}
	bool data = false;
};

// Client <-> server
struct PlayerUpdatePacket
{
	void write(se::WriteBuffer& writeBuffer) const
	{
		se_write(writeBuffer, position);
	}
	bool read(se::ReadBuffer& readBuffer)
	{
		se_read(readBuffer, position);
		return true;
	}
	glm::vec2 position;
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

// Client <-> server
struct BulletCreatePacket
{
	void write(se::WriteBuffer& writeBuffer) const
	{
		se_write(writeBuffer, position2D);
		se_write(writeBuffer, direction2D);
	}
	bool read(se::ReadBuffer& readBuffer)
	{
		se_read(readBuffer, position2D);
		se_read(readBuffer, direction2D);
		return true;
	}
	glm::vec2 position2D;
	glm::vec2 direction2D;
};

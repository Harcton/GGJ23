#pragma once


enum class PacketType
{
	LobbyEnter,
	LobbyReady,
	LobbyStart,
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
		se_write(writeBuffer, message);
	}
	bool read(se::ReadBuffer& readBuffer)
	{
		se_read(readBuffer, message);
		return true;
	}
	std::string message;
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
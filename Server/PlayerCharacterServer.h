#pragma once

struct PlayerUpdatePacket;


class PlayerCharacterServer
{
public:
	PlayerCharacterServer(ServerContext& _context);
	~PlayerCharacterServer();

	void update();

	const std::unordered_map<ClientId, PlayerUpdatePacket>& getPlayerUpdatePackets() const;

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};
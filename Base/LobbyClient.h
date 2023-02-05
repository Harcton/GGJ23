#pragma once

#include "Base/RootStrain.h"
#include "Base/Net/Packets.h"

namespace se
{
	namespace net
	{
		class Connection2;
	}
}

struct LobbyResult
{
	std::shared_ptr<se::net::Connection2> connection;
	ClientId myClientId;
	RootStrain startingRootStrain = RootStrain::Yellow;
	LobbyStartPacket lobbyStartPacket;
};

class LobbyClient
{
public:
	LobbyClient(DemoContext& _context);
	~LobbyClient();

	void update();

	std::optional<LobbyResult> getResult() const;

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};
#pragma once

#include "Server/Client.h"

namespace se
{
	namespace net
	{
		class ConnectionManager2;
	}
}


class LobbyServer
{
public:
	LobbyServer(DemoContext& _context, se::net::ConnectionManager2& _connectionManager);
	~LobbyServer();

	void update();
	void render();

	bool getReadyClients(std::vector<std::unique_ptr<Client>>& deposit);

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};
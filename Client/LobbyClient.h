#pragma once

namespace se
{
	namespace net
	{
		class ConnectionManager2;
		class Connection2;
	}
}


class LobbyClient
{
public:
	LobbyClient(DemoContext& _context, se::net::ConnectionManager2& _connectionManager, const std::string _processFilepath);
	~LobbyClient();

	void update();
	void render();

	std::shared_ptr<se::net::Connection2> getReadyConnection() const;

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};
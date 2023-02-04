#pragma once

namespace se
{
	namespace net
	{
		class ConnectionManager2;
		class Connection2;
	}
}

struct LobbyResult
{
	std::shared_ptr<se::net::Connection2> connection;
	ClientId myClientId;
};

class LobbyClient
{
public:
	LobbyClient(DemoContext& _context, se::net::ConnectionManager2& _connectionManager, const std::string _processFilepath);
	~LobbyClient();

	void update();
	void render();

	std::optional<LobbyResult> getResult() const;

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};
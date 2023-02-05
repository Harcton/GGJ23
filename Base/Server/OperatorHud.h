#pragma once

class PlayerCharacterServer;
class RootServer;


class OperatorHud
{
public:
	OperatorHud(ServerContext& _context, PlayerCharacterServer& _playerCharacterServer, RootServer& _rootServer);
	~OperatorHud();
	void update();
private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};
#pragma once

class PlayerCharacterServer;


class MutatorGui
{
public:
	MutatorGui(ServerContext& _context, PlayerCharacterServer& _playerCharacterServer);
	~MutatorGui();

	void update();

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};
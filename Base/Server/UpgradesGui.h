#pragma once

#include "Base/Server/IOperatorGui.h"

class PlayerCharacterServer;


class UpgradesGui : public IOperatorGui
{
public:
	UpgradesGui(ServerContext& _context, PlayerCharacterServer& _playerCharacterServer);
	~UpgradesGui();

	std::optional<OperatorGui> update() final;

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};
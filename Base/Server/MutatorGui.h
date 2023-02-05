#pragma once

#include "Base/Server/IOperatorGui.h"

class PlayerCharacterServer;


class MutatorGui : public IOperatorGui
{
public:
	MutatorGui(ServerContext& _context, PlayerCharacterServer& _playerCharacterServer);
	~MutatorGui();

	std::optional<OperatorGui> update() final;

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};
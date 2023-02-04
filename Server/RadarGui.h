#pragma once

class PlayerCharacterServer;
class RootServer;


class RadarGui : public IOperatorGui
{
public:
	RadarGui(ServerContext& _context, PlayerCharacterServer& _playerCharacterServer, RootServer& _rootServer);
	~RadarGui();

	std::optional<OperatorGui> update() final;

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};
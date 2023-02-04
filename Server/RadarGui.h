#pragma once

class PlayerCharacterServer;
class RootServer;


class RadarGui
{
public:
	RadarGui(ServerContext& _context, PlayerCharacterServer& _playerCharacterServer, RootServer& _rootServer);
	~RadarGui();

	void update();

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};
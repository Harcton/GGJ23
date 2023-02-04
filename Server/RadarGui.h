#pragma once

class PlayerCharacterServer;


class RadarGui
{
public:
	RadarGui(ServerContext& _context, PlayerCharacterServer& _playerCharacterServer);
	~RadarGui();

	void update();

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};
#pragma once



class PlayerCharacterServer
{
public:
	PlayerCharacterServer(ServerContext& _context);
	~PlayerCharacterServer();

	void update();

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};
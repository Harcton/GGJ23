#pragma once


struct SessionContext;

class RootsGame
{
public:
	RootsGame(ClientContext& _context);
	~RootsGame();
	void update();
private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

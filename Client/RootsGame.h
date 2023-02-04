#pragma once


struct DemoContext;

class RootsGame
{
public:
	RootsGame(DemoContext& _context);
	~RootsGame();
	void update();
private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

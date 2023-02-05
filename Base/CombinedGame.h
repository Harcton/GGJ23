#pragma once


class CombinedGame
{
public:
	CombinedGame(const std::string& _processFilepath, const bool _runClient);
	~CombinedGame();
private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

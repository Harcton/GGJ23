#pragma once



enum class StateTransition { MainMenu, ClientLobby, ServerLobby, Client, Server, Quit };

class CombinedGame
{
public:
	CombinedGame(const std::string& _processFilepath, const std::string& _windowName, const StateTransition _stateTransition = StateTransition::MainMenu);
	~CombinedGame();
private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

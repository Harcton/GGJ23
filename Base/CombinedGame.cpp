#include "stdafx.h"
#include "Base/CombinedGame.h"

#include "Base/Client/RootsGame.h"
#include "Base/ClientUtility/SoundPlayer.h"
#include "Base/DemoContextState.h"
#include "Base/LobbyClient.h"
#include "Base/LobbyServer.h"
#include "Base/MainMenu.h"
#include "Base/Net/Packets.h"
#include "Base/Server/OperatorHud.h"
#include "Base/Server/PacketBroadcaster.h"
#include "Base/Server/PlayerCharacterServer.h"
#include "Base/Server/RootServer.h"
#include "Base/UserSettingsWindow.h"
#include "SpehsEngine/Audio/AudioLib.h"
#include "SpehsEngine/Core/CoreLib.h"
#include "SpehsEngine/Debug/DebugLib.h"
#include "SpehsEngine/Graphics/Window.h"
#include "SpehsEngine/GUI/GUILib.h"
#include "SpehsEngine/Input/InputLib.h"
#include "SpehsEngine/Math/MathLib.h"
#include "SpehsEngine/Net/NetLib.h"
#include "SpehsEngine/Net/Packetman.h"
#include "SpehsEngine/Physics/PhysicsLib.h"


struct CombinedGame::Impl
{
	static constexpr se::time::Time minFrameTime = se::time::fromSeconds(1.0f / float(120.0f));

	Impl(const std::string& _processFilepath, const std::string& _windowName, const StateTransition _stateTransition)
		: processFilepath(_processFilepath)
		, net(core)
		, math(core)
		, physics(math)
		, debug(gui)
		, demoContextState(_windowName, _processFilepath)
		, demoContext(demoContextState.getDemoContext())
	{
		StateTransition stateTransition = _stateTransition;
		if (_stateTransition == StateTransition::ServerLobby)
		{
			demoContext.mainWindow.setX(0);
			demoContext.mainWindow.setY(32);
		}

		while (true)
		{
			switch (stateTransition)
			{
			case StateTransition::Client: stateTransition = runClient(); break;
			case StateTransition::ClientLobby: stateTransition = runClientLobby(); break;
			case StateTransition::Server: stateTransition = runServer(); break;
			case StateTransition::ServerLobby: stateTransition = runServerLobby(); break;
			case StateTransition::Quit: return;
			}
		}
	}

	StateTransition runMainMenu()
	{
		MainMenu mainMenu(demoContext);
		demoContext.soundPlayer.playMusic("main_theme_root_bgm.ogg", se::time::fromSeconds(2.0f));
	}

	StateTransition runClientLobby()
	{
		// Lobby loop
		lobbyResult.reset();

		LobbyClient lobbyClient(demoContext);
		while (true)
		{
			SE_SCOPE_PROFILER("Frame");
			const se::time::ScopedFrameLimiter frameLimiter(minFrameTime);
			
			lobbyClient.update();
			if (!demoContextState.update())
			{
				return StateTransition::Quit;
			}
			lobbyClient.update();
			demoContextState.render();

			lobbyResult = lobbyClient.getResult();
			if (lobbyResult)
			{
				if (lobbyResult->myClientId)
				{
					return StateTransition::Client;
				}
				else
				{
					return StateTransition::MainMenu;
				}
			}
		}
	}

	StateTransition runClient()
	{
		if (!lobbyResult)
		{
			return StateTransition::MainMenu;
		}
		se::net::Packetman<PacketType> sessionPacketman(*lobbyResult->connection);
		ClientContext clientContext
		{
			demoContext,
			sessionPacketman,
			lobbyResult->myClientId,
			lobbyResult->startingRootStrain,
		};
		RootsGame gaem(clientContext);

		// Game loop
		while (true)
		{
			SE_SCOPE_PROFILER("Frame");
			const se::time::ScopedFrameLimiter frameLimiter(minFrameTime);

			if (!demoContextState.update())
			{
				return StateTransition::Quit;
			}

			if (!gaem.update())
			{
				return StateTransition::MainMenu;
			}

			demoContextState.render();
		}
	}

	StateTransition runServerLobby()
	{
		serverClients.clear();

		// Lobby loop
		const se::time::Time minFrameTime = se::time::fromSeconds(1.0f / float(60.0f));
		{
			LobbyServer lobbyServer(demoContext);
			while (true)
			{
				SE_SCOPE_PROFILER("Frame");
				const se::time::ScopedFrameLimiter frameLimiter(minFrameTime);

				if (!demoContextState.update())
				{
					return StateTransition::Quit;
				}
				lobbyServer.update();
				lobbyServer.render();
				demoContextState.render();

				if (lobbyServer.getReadyClients(serverClients))
				{
					return StateTransition::Server;
				}
			}
		}
	}

	StateTransition runServer()
	{
		// Game loop
		ServerContext serverContext
		{
			demoContext,
			serverClients,
		};
		PacketBroadcaster packetBroadcaster(serverContext);
		PlayerCharacterServer playerCharacterServer(serverContext);
		RootServer rootServer(serverContext, constants::worldSize);
		OperatorHud operatorHud(serverContext, playerCharacterServer, rootServer);

		while (true)
		{
			SE_SCOPE_PROFILER("Frame");
			const se::time::ScopedFrameLimiter frameLimiter(minFrameTime);

			rootServer.update();
			operatorHud.update();
			playerCharacterServer.update();
			if (!demoContextState.update())
			{
				return StateTransition::Quit;
			}

			demoContextState.render();

			// TODO: returning to main menu somehow
			//return StateTransition::MainMenu;
		}
	}

	const std::string processFilepath;
	se::CoreLib core;
	se::NetLib net;
	se::MathLib math;
	se::PhysicsLib physics;
	se::AudioLib audio;
	se::InputLib input;
	se::GUILib gui;
	se::debug::DebugLib debug;
	DemoContextState demoContextState;
	DemoContext demoContext;
	std::optional<LobbyResult> lobbyResult;
	std::vector<std::unique_ptr<Client>> serverClients;
};

CombinedGame::CombinedGame(const std::string& _processFilepath, const std::string& _windowName, const StateTransition _stateTransition)
	: impl(new Impl(_processFilepath, _windowName, _stateTransition))
{
}

CombinedGame::~CombinedGame()
{
	// ~Impl()
}
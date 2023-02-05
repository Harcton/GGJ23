#include "stdafx.h"
#include "Base/CombinedGame.h"

#include "Base/Client/ClientHud.h"
#include "Base/Client/RootsGame.h"
#include "Base/ClientUtility/SoundPlayer.h"
#include "Base/DemoContextState.h"
#include "Base/LobbyClient.h"
#include "Base/LobbyServer.h"
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
#include "SpehsEngine/Net/ConnectionManager2.h"
#include "SpehsEngine/Net/NetLib.h"
#include "SpehsEngine/Net/Packetman.h"
#include "SpehsEngine/Physics/PhysicsLib.h"



struct CombinedGame::Impl
{
	Impl(const std::string& _processFilepath, const bool _runClient)
		: processFilepath(_processFilepath)
		, net(core)
		, math(core)
		, physics(math)
		, debug(gui)
		, demoContextState("Client")
		, demoContext(demoContextState.getDemoContext())
	{
		if (_runClient)
		{
			runClient();
		}
		else
		{
			runServer();
		}
	}

	void runClient()
	{
		constexpr se::time::Time minFrameTime = se::time::fromSeconds(1.0f / float(120.0f));

		se::net::ConnectionManager2 connectionManager("Client");
		UserSettingsWindow userSettingsWindow(demoContext);
		ClientHud clientHud(demoContext, userSettingsWindow);

		// Lobby loop
		std::optional<LobbyResult> lobbyResult;

		demoContext.soundPlayer.playMusic("main_theme_root_bgm.ogg", se::time::fromSeconds(2.0f));

		{
			LobbyClient lobbyClient(demoContext, connectionManager, processFilepath);
			while (true)
			{
				SE_SCOPE_PROFILER("Frame");
				const se::time::ScopedFrameLimiter frameLimiter(minFrameTime);

				connectionManager.update();
				if (!demoContextState.update())
				{
					return;
				}
				lobbyClient.update();
				lobbyClient.render();
				demoContextState.render();

				lobbyResult = lobbyClient.getResult();
				if (lobbyResult)
				{
					break;
				}
			}
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

			connectionManager.update();
			if (!demoContextState.update())
			{
				break;
			}

			gaem.update();
			clientHud.update();

			demoContextState.render();
		}
	}

	void runServer()
	{
		demoContext.mainWindow.setX(0);
		demoContext.mainWindow.setY(32);

		se::ScopedConnections scopedConnections;
		se::net::ConnectionManager2 connectionManager("server");
		UserSettingsWindow userSettingsWindow(demoContext);

		// Lobby loop
		const se::time::Time minFrameTime = se::time::fromSeconds(1.0f / float(60.0f));
		std::vector<std::unique_ptr<Client>> clients;
		{
			LobbyServer lobbyServer(demoContext, connectionManager);
			while (true)
			{
				SE_SCOPE_PROFILER("Frame");
				const se::time::ScopedFrameLimiter frameLimiter(minFrameTime);

				connectionManager.update();
				if (!demoContextState.update())
				{
					break;
				}
				lobbyServer.update();
				lobbyServer.render();
				demoContextState.render();

				if (lobbyServer.getReadyClients(clients))
				{
					break;
				}
			}
		}

		// Game loop
		ServerContext serverContext
		{
			demoContext,
			clients,
		};
		PacketBroadcaster packetBroadcaster(serverContext);
		PlayerCharacterServer playerCharacterServer(serverContext);
		RootServer rootServer(serverContext, constants::worldSize);
		OperatorHud operatorHud(serverContext, playerCharacterServer, rootServer);

		while (true)
		{
			SE_SCOPE_PROFILER("Frame");
			const se::time::ScopedFrameLimiter frameLimiter(minFrameTime);

			connectionManager.update();
			rootServer.update();
			operatorHud.update();
			playerCharacterServer.update();
			if (!demoContextState.update())
			{
				break;
			}

			demoContextState.render();
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
};

CombinedGame::CombinedGame(const std::string& _processFilepath, const bool _runClient)
	: impl(new Impl(_processFilepath, _runClient))
{
}

CombinedGame::~CombinedGame()
{
	// ~Impl()
}
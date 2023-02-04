#include "stdafx.h"

#include "SpehsEngine/Audio/AudioLib.h"
#include "SpehsEngine/Core/CoreLib.h"
#include "SpehsEngine/Math/MathLib.h"
#include "SpehsEngine/Net/NetLib.h"
#include "SpehsEngine/Net/ConnectionManager2.h"
#include "SpehsEngine/Input/InputLib.h"
#include "SpehsEngine/Physics/PhysicsLib.h"
#include "SpehsEngine/Graphics/Window.h"
#include "SpehsEngine/GUI/GUILib.h"
#include "SpehsEngine/Debug/DebugLib.h"
#include "Base/DemoContextState.h"
#include "Base/UserSettingsWindow.h"
#include "Server/LobbyServer.h"
#include "Server/PacketBroadcaster.h"
#include "Server/PlayerCharacterServer.h"
#include "Server/RootServer.h"
#include "Server/RadarGui.h"
#include "Server/MutatorGui.h"


int main()
{
	se::CoreLib core;
	se::NetLib net(core);
	se::MathLib math(core);
	se::PhysicsLib physics(math);
	se::AudioLib audio;
	se::InputLib input;
	se::GUILib gui;
	se::debug::DebugLib debug(gui);

	DemoContextState demoContextState("Server");
	DemoContext demoContext = demoContextState.getDemoContext();

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
	RadarGui radarGui(serverContext, playerCharacterServer, rootServer);
	MutatorGui mutatorGui(serverContext, playerCharacterServer);
	while (true)
	{
		SE_SCOPE_PROFILER("Frame");
		const se::time::ScopedFrameLimiter frameLimiter(minFrameTime);

		connectionManager.update();
		radarGui.update();
		rootServer.update();
		playerCharacterServer.update();
		if (!demoContextState.update())
		{
			break;
		}

		demoContextState.render();
	}

	return 0;
}

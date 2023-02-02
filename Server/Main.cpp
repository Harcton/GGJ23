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
#include "Server/LobbyServer.h"


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

	demoContextState.showWindowDefault(glm::ivec2(1280, 720));
	demoContext.mainWindow.setX(0);
	demoContext.mainWindow.setY(32);

	se::ScopedConnections scopedConnections;
	se::net::ConnectionManager2 connectionManager("server");

	// Lobby loop
	const se::time::Time minFrameTime = se::time::fromSeconds(1.0f / float(60.0f));
	std::vector<Client> clients;
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
	while (true)
	{
		SE_SCOPE_PROFILER("Frame");
		const se::time::ScopedFrameLimiter frameLimiter(minFrameTime);

		connectionManager.update();
		if (!demoContextState.update())
		{
			break;
		}

		if (ImGui::Begin("Server game"))
		{
		}
		ImGui::End();

		demoContextState.render();
	}

	return 0;
}

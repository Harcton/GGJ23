#include "stdafx.h"

#include "SpehsEngine/Audio/AudioLib.h"
#include "SpehsEngine/Core/CoreLib.h"
#include "SpehsEngine/Math/MathLib.h"
#include "SpehsEngine/Net/ConnectionManager2.h"
#include "SpehsEngine/Net/NetLib.h"
#include "SpehsEngine/Input/InputLib.h"
#include "SpehsEngine/Physics/PhysicsLib.h"
#include "SpehsEngine/GUI/GUILib.h"
#include "SpehsEngine/Debug/DebugLib.h"
#include "Base/DemoContextState.h"
#include "Client/LobbyClient.h"
#include "Client/RootsGame.h"


int main(const int argc, const char** argv)
{
	se::CoreLib core;
	se::NetLib net(core);
	se::MathLib math(core);
	se::PhysicsLib physics(math);
	se::AudioLib audio;
	se::InputLib input;
	se::GUILib gui;
	se::debug::DebugLib debug(gui);

	DemoContextState demoContextState("Client");
	DemoContext demoContext = demoContextState.getDemoContext();
	(void)demoContext;

	demoContextState.showWindowDefault(glm::ivec2(1920, 1080));
	constexpr se::time::Time minFrameTime = se::time::fromSeconds(1.0f / float(120.0f));

	se_assert(argc > 0);
	const std::string processFilepath = argv[0];
	se::net::ConnectionManager2 connectionManager("Client");

	// Lobby loop
	std::shared_ptr<se::net::Connection2> connection;

	if (false)
	{
		LobbyClient lobbyClient(demoContext, connectionManager, processFilepath);
		while (true)
		{
			SE_SCOPE_PROFILER("Frame");
			const se::time::ScopedFrameLimiter frameLimiter(minFrameTime);

			connectionManager.update();
			if (!demoContextState.update())
			{
				return 0;
			}
			lobbyClient.update();
			lobbyClient.render();
			demoContextState.render();

			connection = lobbyClient.getReadyConnection();
			if (connection)
			{
				break;
			}
		}
	}

	RootsGame gaem(demoContext);

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

		if (ImGui::Begin("Client game"))
		{

		}
		ImGui::End();

		demoContextState.render();
	}

	return 0;
}

#include "stdafx.h"

#include "SpehsEngine/Audio/AudioLib.h"
#include "SpehsEngine/Core/CoreLib.h"
#include "SpehsEngine/Math/MathLib.h"
#include "SpehsEngine/Net/ConnectionManager2.h"
#include "SpehsEngine/Net/NetLib.h"
#include "SpehsEngine/Net/Packetman.h"
#include "SpehsEngine/Input/InputLib.h"
#include "SpehsEngine/Physics/PhysicsLib.h"
#include "SpehsEngine/GUI/GUILib.h"
#include "SpehsEngine/Debug/DebugLib.h"
#include "Base/DemoContextState.h"
#include "Base/UserSettingsWindow.h"
#include "Base/Net/Packets.h"
#include "Base/ClientUtility/SoundPlayer.h"
#include "Client/ClientHud.h"
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

	constexpr se::time::Time minFrameTime = se::time::fromSeconds(1.0f / float(120.0f));

	se_assert(argc > 0);
	const std::string processFilepath = argv[0];
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
				return 0;
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

	return 0;
}

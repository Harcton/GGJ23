#include "stdafx.h"
#include "Base/LobbyClient.h"

#include "Base/Net/Packets.h"
#include "SpehsEngine/Core/OS.h"
#include "SpehsEngine/Net/ConnectionManager2.h"
#include "SpehsEngine/Net/Packetman.h"


struct LobbyClient::Impl
{
	Impl(DemoContext& _context)
		: context(_context)
	{
		name = "Player #" + std::to_string(se::rng::random<uint16_t>());
		context.imguiBackend.connectToPreRenderSignal(scopedConnections.add(), [this]()
			{
				render();
			});
	}

	void update()
	{
		if (packetman)
		{
			packetman->update();
		}
	}

	void render()
	{
		const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;
		if (ImGui::BeginCentered("LobbyClient", nullptr, windowFlags, ImGuiCond_Always))
		{
			if (connection)
			{
				switch (connection->getStatus())
				{
				case se::net::Connection2::Status::Connecting:
					ImGui::Text("Connecting");
					break;
				case se::net::Connection2::Status::Connected:
					ImGui::Text("Connected");
					if (ImGui::InputT("Ready", ready))
					{
						sendReady();
					}
					break;
				case se::net::Connection2::Status::Disconnected:
					ImGui::Text("Disconnected");
					break;
				}
			}
			else
			{
				ImGui::InputT("Name", name);
				ImGui::InputT("Server address", address);
				if (ImGui::Button("Connect to server") || context.userSettings.getSkipLobby())
				{
					connection = context.connectionManager.connectIP(se::net::Endpoint(se::net::Address(address), se::net::Port(41623)), false, "Client");
					if (connection)
					{
						connection->connectToStatusChangedSignal(scopedConnections.add(),
							[&](const se::net::Connection2::Status oldStatus, const se::net::Connection2::Status newStatus)
							{
								switch (newStatus)
								{
								case se::net::Connection2::Status::Connected:
								{
									packetman.reset(new se::net::Packetman<PacketType>(*connection));
									LobbyEnterPacket packet;
									packet.name = name;
									packetman->sendPacket<LobbyEnterPacket, LobbyEnterResult>(PacketType::LobbyEnter, packet, scopedConnections.add(),
										[this](LobbyEnterResult* const _result)
										{
											if (_result)
											{
												se::log::info("EnterLobbyResult my client id: " + std::to_string(_result->clientId.value));
												myClientId = _result->clientId;
												startingRootStrain = _result->startingRootStrain;
												packetman->registerReceiveHandler<LobbyStartPacket>(PacketType::LobbyStart, scopedConnections.add(),
													[this](LobbyStartPacket& _packet, const bool _reliable)
													{
														startRequested = true;
													});
												if (context.userSettings.getSkipLobby())
												{
													ready = true;
													sendReady();
												}
											}
											else
											{
												se::log::info("EnterLobbyResult failed");
											}
										});
								}
								break;
								case se::net::Connection2::Status::Connecting:
									break;
								case se::net::Connection2::Status::Disconnected:
									break;
								}
							});
					}
				}
			}
			if (ImGui::Button("Launch another client"))
			{
				se::createProcess(context.processFilepath, "");
			}
		}
	}

	void sendReady()
	{
		LobbyReadyPacket packet;
		packet.ready = ready;
		packetman->sendPacket(PacketType::LobbyReady, packet, true);
	}

	std::optional<LobbyResult> getResult() const
	{
		if (startRequested)
		{
			LobbyResult result;
			result.connection = connection;
			result.myClientId = myClientId;
			result.startingRootStrain = startingRootStrain;
			return result;
		}
		else
		{
			return std::nullopt;
		}
	}

	DemoContext& context;
	se::ScopedConnections scopedConnections;
	std::shared_ptr<se::net::Connection2> connection;
	std::unique_ptr<se::net::Packetman<PacketType>> packetman;
	std::string name;
	std::string address = "127.0.0.1";
	ClientId myClientId;
	RootStrain startingRootStrain = RootStrain::Blue;
	bool ready = false;
	bool startRequested = false;
};

LobbyClient::LobbyClient(DemoContext& _context)
	: impl(new Impl(_context))
{
}

LobbyClient::~LobbyClient()
{
	// ~Impl()
}

void LobbyClient::update()
{
	impl->update();
}

std::optional<LobbyResult> LobbyClient::getResult() const
{
	return impl->getResult();
}

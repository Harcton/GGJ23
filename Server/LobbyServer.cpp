#include "stdafx.h"
#include "Server/LobbyServer.h"

#include "Base/Net/Packets.h"
#include "Server/Client.h"
#include "SpehsEngine/Net/ConnectionManager2.h"
#include "SpehsEngine/Net/Packetman.h"
#pragma optimize("", off)

struct LobbyServer::Impl
{
	struct LobbyClient
	{
		LobbyClient(const ClientId _clientId, const std::string_view _name, const std::shared_ptr<se::net::Connection2>& _connection)
			: packetman(*_connection)
		{
			client.connection = _connection;
			client.clientId = _clientId;
			client.name = _name;
		}
		Client client;
		se::net::Packetman<PacketType> packetman;
		boost::signals2::scoped_connection enterConnection;
		boost::signals2::scoped_connection readyConnection;
		bool ready = false;
	};
	Impl(DemoContext& _context, se::net::ConnectionManager2& _connectionManager)
		: context(_context)
		, connectionManager(_connectionManager)
	{
		connectionManager.startAcceptingIP(se::net::Port(41623));
		connectionManager.connectToIncomingConnectionSignal(scopedConnections.add(),
			[this](std::shared_ptr<se::net::Connection2>& _connection)
			{
				if (clients.size() + connectingClients.size() >= targetClientCount)
				{
					_connection->disconnect();
					return;
				}
				const ClientId clientId(nextClientId.value++);
				connectingClients.push_back(std::make_unique<LobbyClient>(clientId, _connection->name,  _connection));
				connectingClients.back()->packetman.registerReceiveHandler<LobbyEnterPacket, LobbyEnterResult>(
					PacketType::LobbyEnter, connectingClients.back()->enterConnection,
					[this, clientId](LobbyEnterPacket& _packet, const bool _reliable)->LobbyEnterResult
					{
						LobbyEnterResult result;
						for (size_t i = 0; i < connectingClients.size(); i++)
						{
							if (connectingClients[i]->client.clientId == clientId)
							{
								std::unique_ptr<LobbyClient> client;
								std::swap(client, connectingClients[i]);
								connectingClients[i].swap(connectingClients.back());
								connectingClients.pop_back();
								if (validateName(_packet.name))
								{
									client->client.name = _packet.name;
									addClient(std::move(client));
								}
								else
								{
									se::log::warning("Client name validation failed: " + _packet.name);
								}
								result.message = "welcome";
								return result;
							}
						}
						result.message = "Unknown client";
						return result;
					});
			});
	}

	void addClient(std::unique_ptr<LobbyClient>&& _client)
	{
		LobbyClient* const client = _client.get();
		clients.push_back(std::move(_client));
		clients.back()->packetman.registerReceiveHandler<LobbyReadyPacket>(
			PacketType::LobbyReady, client->readyConnection,
			[this, client](LobbyReadyPacket& _packet, const bool _reliable)
			{
				client->ready = _packet.ready;
			});
	}

	void update()
	{
		for (const std::unique_ptr<LobbyClient>& client : connectingClients)
		{
			client->packetman.update();
		}
		for (const std::unique_ptr<LobbyClient>& client : clients)
		{
			client->packetman.update();
		}
	}

	void render()
	{
		if (ImGui::Begin("Lobby"))
		{
			if (ImGui::InputT("Target client count", targetClientCount))
			{
				if (clients.size() > targetClientCount)
				{
					clients.resize(targetClientCount);
				}
				const uint8_t remaining = targetClientCount - uint8_t(clients.size());
				if (connectingClients.size() > remaining)
				{
					connectingClients.resize(remaining);
				}
			}
			ImGui::Text("Clients (" + std::to_string(clients.size()) + ")");
			ImGui::Indent();
			for (const std::unique_ptr<LobbyClient>& client : clients)
			{
				std::string string = client->client.name;
				if (client->ready)
				{
					string += " (ready)";
				}
				ImGui::Text(string);
			}
			ImGui::Unindent();

			ImGui::Text("Connecting clients (" + std::to_string(connectingClients.size()) + ")");
			ImGui::Indent();
			for (const std::unique_ptr<LobbyClient>& client : connectingClients)
			{
				ImGui::Text(client->client.name.empty() ? client->client.connection->remoteEndpoint.toString() : client->client.name);
			}
			ImGui::Unindent();
		}
	}

	bool validateName(const std::string& name) const
	{
		if (name.empty())
		{
			return false;
		}
		else if (name.size() > 32)
		{
			return false;
		}
		for (const std::unique_ptr<LobbyClient>& client : clients)
		{
			if (client->client.name == name)
			{
				return false;
			}
		}
		return true;
	}

	bool getReadyClients(std::vector<Client>& deposit) const
	{
		if (clients.size() != targetClientCount)
		{
			return false;
		}
		for (const std::unique_ptr<LobbyClient>& client : clients)
		{
			if (!client->ready)
			{
				return false;
			}
		}
		LobbyStartPacket packet;
		for (const std::unique_ptr<LobbyClient>& client : clients)
		{
			client->packetman.sendPacket(PacketType::LobbyStart, packet, true);
			deposit.push_back(client->client);
		}
		return true;
	}

	DemoContext& context;
	se::net::ConnectionManager2& connectionManager;
	uint8_t targetClientCount = 1;
	se::ScopedConnections scopedConnections;
	std::vector<std::unique_ptr<LobbyClient>> connectingClients;
	std::vector<std::unique_ptr<LobbyClient>> clients;
	ClientId nextClientId = ClientId(1);
};

LobbyServer::LobbyServer(DemoContext& _context, se::net::ConnectionManager2& _connectionManager)
	: impl(new Impl(_context, _connectionManager))
{
}

LobbyServer::~LobbyServer()
{
	// ~Impl()
}

void LobbyServer::update()
{
	impl->update();
}

void LobbyServer::render()
{
	impl->render();
}

bool LobbyServer::getReadyClients(std::vector<Client>& deposit) const
{
	return impl->getReadyClients(deposit);
}
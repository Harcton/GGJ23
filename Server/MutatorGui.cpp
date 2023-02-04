#include "stdafx.h"
#include "Server/MutatorGui.h"

#include "Base/Net/Packets.h"
#include "Base/MutationDatabase.h"
#include "Server/PlayerCharacterServer.h"
#include "Server/RootServer.h"
#include "SpehsEngine/GUI/GUIShape.h"
#include "SpehsEngine/GUI/GUIView.h"
#pragma optimize("", off)

using namespace se::gui;
using namespace se::gui::unit_literals;


struct MutatorGui::Impl
{
	Impl(ServerContext& _context, PlayerCharacterServer& _playerCharacterServer)
		: context(_context)
		, playerCharacterServer(_playerCharacterServer)
	{
		context.imguiBackend.connectToPreRenderSignal(scopedConnections.add(),
			[this]()
			{
				if (ImGui::Begin("Mutator"))
				{
					if (client)
					{
						for (const Mutation* const mutation : context.mutationDatabase.vector)
						{
							if (ImGui::Button(mutation->name))
							{
								PlayerMutatePacket packet;
								packet.mutationId = mutation->mutationId;
								packet.stacks = 1;
								client->packetman.sendPacket(PacketType::PlayerMutated, packet, true);
							}
						}
					}
					else
					{
						for (const std::unique_ptr<Client>& c : context.clients)
						{
							if (ImGui::Button(c->name))
							{
								client = c.get();
							}
						}
					}
				}
				ImGui::End();
			});
	}

	~Impl()
	{
	}

	void update()
	{

	}

	ServerContext& context;
	PlayerCharacterServer& playerCharacterServer;
	se::ScopedConnections scopedConnections;
	Client* client = nullptr;
};

MutatorGui::MutatorGui(ServerContext& _context, PlayerCharacterServer& _playerCharacterServer)
	: impl(new Impl(_context, _playerCharacterServer))
{
}

MutatorGui::~MutatorGui()
{
	// ~Impl()
}

void MutatorGui::update()
{
	impl->update();
}

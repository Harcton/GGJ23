#include "stdafx.h"
#include "Server/MutatorGui.h"

#include "Base/Net/Packets.h"
#include "Base/MutationDatabase.h"
#include "Server/PlayerCharacterServer.h"
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
						ImGui::Text("Select loadout:");
						ImGui::Indent();
						for (const Mutation* const mutation : context.mutationDatabase.vector)
						{
							if (mutation->mutationCategory != MutationCategory::Loadout)
							{
								continue;
							}
							if (ImGui::Button(mutation->name))
							{
								PlayerMutatePacket packet;
								packet.mutationId = mutation->mutationId;
								packet.stacks = 1;
								client->packetman.sendPacket(PacketType::PlayerMutated, packet, true);
							}
						}
						ImGui::Unindent();
						if (context.userSettings.getDevMode())
						{
							ImGui::Separator();
							ImGui::Text("DEV:");
							ImGui::Indent();
							for (const Mutation* const mutation : context.mutationDatabase.vector)
							{
								if (mutation->mutationCategory == MutationCategory::Loadout)
								{
									continue;
								}
								if (ImGui::Button(mutation->name))
								{
									PlayerMutatePacket packet;
									packet.mutationId = mutation->mutationId;
									packet.stacks = 1;
									client->packetman.sendPacket(PacketType::PlayerMutated, packet, true);
								}
							}
							ImGui::Unindent();
						}
						ImGui::Separator();
						if (ImGui::Button("Back"))
						{
							client = nullptr;
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

	std::optional<OperatorGui> update()
	{
		return nextOperatorGui;
	}

	ServerContext& context;
	PlayerCharacterServer& playerCharacterServer;
	se::ScopedConnections scopedConnections;
	Client* client = nullptr;
	std::optional<OperatorGui> nextOperatorGui;
};

MutatorGui::MutatorGui(ServerContext& _context, PlayerCharacterServer& _playerCharacterServer)
	: impl(new Impl(_context, _playerCharacterServer))
{
}

MutatorGui::~MutatorGui()
{
	// ~Impl()
}

std::optional<IOperatorGui::OperatorGui> MutatorGui::update()
{
	return impl->update();
}

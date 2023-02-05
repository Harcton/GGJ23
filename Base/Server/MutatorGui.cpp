#include "stdafx.h"
#include "Base/Server/MutatorGui.h"

#include "Base/Net/Packets.h"
#include "Base/MutationDatabase.h"
#include "Base/Server/PlayerCharacterServer.h"
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
				if (!client)
				{
					client = context.clients.front().get();
				}

				const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove;

				ImGui::SetNextWindowPos(ImVec2(
					float(context.mainWindow.getWidth()) * 0.5f,
					float(context.mainWindow.getHeight()) * 0.5f), 0, ImVec2(1.1f, 0.5f));
				if (ImGui::Begin("Mutator clients", nullptr, windowFlags))
				{
					ImGui::Text("Pilot selection");
					ImGui::Indent();
					for (const std::unique_ptr<Client>& c : context.clients)
					{
						if (c.get() == client)
						{
							ImGui::Text("> " + client->name);
						}
						else if (ImGui::Button(c->name))
						{
							client = c.get();
						}
					}
					ImGui::Unindent();
				}
				ImGui::End();

				ImGui::SetNextWindowPos(ImVec2(
					float(context.mainWindow.getWidth()) * 0.5f,
					float(context.mainWindow.getHeight()) * 0.5f), 0, ImVec2(-0.1f, 0.5f));
				if (ImGui::Begin("Mutator", nullptr, windowFlags))
				{
					if (client)
					{
						ImGui::Text("Select loadout:");
						ImGui::Indent();
						for (const Mutation* const mutation : context.mutationDatabase.vector)
						{
							if (!mutation->rootStrain)
							{
								continue;
							}
							else if (client->rootStrainLoadout == *mutation->rootStrain)
							{
								ImGui::Text("> " + mutation->name);
							}
							else if (ImGui::Button(mutation->name))
							{
								client->rootStrainLoadout = *mutation->rootStrain;
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
								if (mutation->rootStrain)
								{
									continue;
								}
								else if (ImGui::Button(mutation->name))
								{
									PlayerMutatePacket packet;
									packet.mutationId = mutation->mutationId;
									packet.stacks = 1;
									client->packetman.sendPacket(PacketType::PlayerMutated, packet, true);
								}
							}
							ImGui::Unindent();
						}
					}
				}
				ImGui::End();
			});
	}

	std::optional<OperatorGui> update()
	{
		return std::nullopt;
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

std::optional<IOperatorGui::OperatorGui> MutatorGui::update()
{
	return impl->update();
}

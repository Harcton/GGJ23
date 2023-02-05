#include "stdafx.h"
#include "Base/Server/UpgradesGui.h"

#include "Base/RenderTopLeftHelpTooltip.h"
#include "Base/ClientUtility/SoundPlayer.h"
#include "Base/Net/Packets.h"
#include "Base/MutationDatabase.h"
#include "Base/Server/PlayerCharacterServer.h"
#include "SpehsEngine/GUI/GUIShape.h"
#include "SpehsEngine/GUI/GUIView.h"
#pragma optimize("", off)

using namespace se::gui;
using namespace se::gui::unit_literals;


struct UpgradesGui::Impl
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
				if (ImGui::Begin("Upgrade clients", nullptr, windowFlags))
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
				if (ImGui::Begin("UpgradesGui", nullptr, windowFlags))
				{
					if (client)
					{
						ImGui::Text("Money: " + std::to_string(context.money));
						ImGui::Text("Select upgrade:");
						ImGui::Indent();
						for (const Mutation* const mutation : context.mutationDatabase.vector)
						{
							if (mutation->rootStrain)
							{
								continue;
							}
							const std::string string = mutation->name + " " + std::to_string(mutation->cost);
							if (context.money < mutation->cost)
							{
								ImGui::Text(string);
							}
							else if (ImGui::Button(string))
							{
								context.soundPlayer.playSound("upgrade_purchase.ogg", glm::vec3());
								context.money -= mutation->cost;
								PlayerMutatePacket packet;
								packet.mutationId = mutation->mutationId;
								packet.stacks = 1;
								client->packetman.sendPacket(PacketType::PlayerMutated, packet, true);
							}
						}
						ImGui::Unindent();
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

UpgradesGui::UpgradesGui(ServerContext& _context, PlayerCharacterServer& _playerCharacterServer)
	: impl(new Impl(_context, _playerCharacterServer))
{
}

UpgradesGui::~UpgradesGui()
{
	// ~Impl()
}

std::optional<IOperatorGui::OperatorGui> UpgradesGui::update()
{
	return impl->update();
}

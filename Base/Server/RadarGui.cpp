#include "stdafx.h"
#include "Base/Server/RadarGui.h"

#include "Base/Net/Packets.h"
#include "Base/Server/PlayerCharacterServer.h"
#include "Base/Server/RootServer.h"
#include "SpehsEngine/GUI/GUIShape.h"
#include "SpehsEngine/GUI/GUIView.h"
#pragma optimize("", off)

using namespace se::gui;
using namespace se::gui::unit_literals;


struct RadarGui::Impl
{
	Impl(ServerContext& _context, PlayerCharacterServer& _playerCharacterServer, RootServer& _rootServer)
		: context(_context)
		, playerCharacterServer(_playerCharacterServer)
		, rootServer(_rootServer)
		, rootShape(se::graphics::ShapeType::Circle)
	{
		context.guiView.add(rootShape);
		rootShape.setPosition(GUIVec2(0.5_view));
		rootShape.setAnchor(0.5_self);
		rootShape.setSize(GUIVec2(0.5_vh));
		rootShape.setColor(se::Color(0.1f, 0.1f, 0.1f));
		for (std::unique_ptr<Client>& client : context.clients)
		{
			GUIShape& shape = rootShape.addChild<se::gui::GUIShape>();
			playerCharacterShapes[client->clientId] = &shape;
		}
		context.imguiBackend.connectToPreRenderSignal(scopedConnections.add(),
			[this]()
			{
				if (!context.userSettings.getDevMode())
				{
					return;
				}
				if (ImGui::Begin("Radar DEV"))
				{
					ImGui::InputT("Damage", devDamage);
					for (const RootServer::Root& root : rootServer.getRoots())
					{
						renderRootDev(root);
					};
				}
				ImGui::End();
			});
	}

	~Impl()
	{
		context.guiView.remove(rootShape);
	}

	GUIUnit sceneToGuiLength(const float length) const
	{
		return GUIUnit(length / constants::worldSize, GUIUnitType::Parent);
	}

	GUIVec2 sceneToGuiPosition(const glm::vec2& scenePosition) const
	{
		return GUIVec2(glm::vec2(scenePosition.x, scenePosition.y) / constants::worldSize + glm::vec2(0.5f, 0.5f), GUIUnitType::Parent);
	}

	std::optional<OperatorGui> update()
	{
		for (const std::pair<const ClientId, PlayerUpdatePacket>& pair : playerCharacterServer.getPlayerUpdatePackets())
		{
			playerCharacterShapes[pair.first]->setPosition(sceneToGuiPosition(pair.second.position));
		}
		std::unordered_set<RootId> referencedRootIds;
		referencedRootIds.reserve(rootServer.getRoots().size());
		rootServer.forEachRoot([&](const RootServer::Root& root)
			{
				GUIShape** existingShape = tryFind(rootShapes, root.rootId);
				GUIShape& shape = existingShape ? **existingShape : createRootShape(root);
				referencedRootIds.insert(root.rootId);
			});
		for (std::unordered_map<RootId, se::gui::GUIShape*>::iterator it = rootShapes.begin(); it != rootShapes.end();)
		{
			if (tryFind(referencedRootIds, it->first))
			{
				it++;
			}
			else
			{
				rootShape.removeChild(it->second);
				it = rootShapes.erase(it);
			}
		}
		return nextOperatorGui;
	}

	void renderRootDev(const RootServer::Root& _root)
	{
		ImGui::PushID(&_root);
		ImGui::Text(std::to_string(_root.rootId.value) + " hp: " + std::to_string(int(_root.health)));
		ImGui::SameLine();
		if (ImGui::Button("Damage"))
		{
			RootDamagePacket packet;
			packet.rootId = _root.rootId;
			packet.damage = devDamage;
			rootServer.apply(packet);
		}
		ImGui::Indent();
		for (const RootServer::Root& child : _root.children)
		{
			renderRootDev(child);
		}
		ImGui::Unindent();
	}

	GUIShape& createRootShape(const RootServer::Root& _root)
	{
		const float sceneLength = glm::length(_root.start - _root.end);
		const glm::vec2 sceneCenter = 0.5f * (_root.start + _root.end);
		const GUIVec2 guiCenter = sceneToGuiPosition(sceneCenter);
		const GUIUnit guiLength = sceneToGuiLength(sceneLength);
		const GUIVec2 size(guiLength, guiLength);
		GUIShape& shape = rootShape.addChild<GUIShape>();
		shape.setPosition(guiCenter);
		shape.setSize(size);
		shape.setColor(toColor(_root.rootStrain));
		rootShapes[_root.rootId] = &shape;
		return shape;
	}

	ServerContext& context;
	PlayerCharacterServer& playerCharacterServer;
	RootServer& rootServer;
	float devDamage = 100.0f;
	se::ScopedConnections scopedConnections;
	se::gui::GUIShape rootShape;
	std::unordered_map<ClientId, se::gui::GUIShape*> playerCharacterShapes;
	std::unordered_map<RootId, se::gui::GUIShape*> rootShapes;
	std::optional<OperatorGui> nextOperatorGui;
};

RadarGui::RadarGui(ServerContext& _context, PlayerCharacterServer& _playerCharacterServer, RootServer& _rootServer)
	: impl(new Impl(_context, _playerCharacterServer, _rootServer))
{
}

RadarGui::~RadarGui()
{
	// ~Impl()
}

std::optional<IOperatorGui::OperatorGui> RadarGui::update()
{
	return impl->update();
}

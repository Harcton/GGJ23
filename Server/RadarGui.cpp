#include "stdafx.h"
#include "Server/RadarGui.h"

#include "Base/Net/Packets.h"
#include "Server/PlayerCharacterServer.h"
#include "SpehsEngine/GUI/GUIShape.h"
#include "SpehsEngine/GUI/GUIView.h"
#pragma optimize("", off)

using namespace se::gui;
using namespace se::gui::unit_literals;


struct RadarGui::Impl
{
	Impl(ServerContext& _context, PlayerCharacterServer& _playerCharacterServer)
		: context(_context)
		, playerCharacterServer(_playerCharacterServer)
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
	}

	~Impl()
	{
		context.guiView.remove(rootShape);
	}

	void update()
	{
		for (const std::pair<const ClientId, PlayerUpdatePacket>& pair : playerCharacterServer.getPlayerUpdatePackets())
		{
			GUIVec2 position(glm::vec2(pair.second.position.x, pair.second.position.y) / constants::worldSize + glm::vec2(0.5f, 0.5f), GUIUnitType::Parent);
			playerCharacterShapes[pair.first]->setPosition(position);
		}
	}

	ServerContext& context;
	PlayerCharacterServer& playerCharacterServer;
	se::ScopedConnections scopedConnections;
	se::gui::GUIShape rootShape;
	std::unordered_map<ClientId, se::gui::GUIShape*> playerCharacterShapes;
};

RadarGui::RadarGui(ServerContext& _context, PlayerCharacterServer& _playerCharacterServer)
	: impl(new Impl(_context, _playerCharacterServer))
{
}

RadarGui::~RadarGui()
{
	// ~Impl()
}

void RadarGui::update()
{
	impl->update();
}

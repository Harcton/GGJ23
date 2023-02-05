#include "stdafx.h"
#include "Base/Server/OperatorHud.h"

#include "SpehsEngine/GUI/GuiView.h"
#include "SpehsEngine/GUI/GUIShape.h"
#include "SpehsEngine/GUI/GUIText.h"
#include "Base/Server/RadarGui.h"
#include "Base/Server/MutatorGui.h"
#include "Base/Server/MonitorGui.h"
#pragma optimize("", off)

using namespace se::gui;
using namespace se::gui::unit_literals;


struct OperatorHud::Impl
{
	Impl(ServerContext& _context, PlayerCharacterServer& _playerCharacterServer, RootServer& _rootServer)
		: context(_context)
		, playerCharacterServer(_playerCharacterServer)
		, rootServer(_rootServer)
		, backShape(backgroundShape.addChild<GUIShape>())
	{
		backgroundShape.setZIndex(-100);
		backgroundShape.setPosition(0.5_view);
		backgroundShape.setAnchor(0.5_self);
		backgroundShape.setSize(1.0_view);
		backgroundShape.setColor(se::Color(0.2f, 0.22f, 0.2f));
		context.guiView.add(backgroundShape);
		{
			GUIText& text = backShape.addChild<se::gui::GUIText>();
			text.insert("BACK");
			text.setPosition(se::gui::GUIVec2(glm::vec2(0.5f, 0.5f), se::gui::GUIUnitType::Parent));
			text.setAnchor(se::gui::GUIVec2(glm::vec2(0.5f, 0.5f), se::gui::GUIUnitType::Self));
			text.setSize(se::gui::GUIVec2(se::gui::GUIUnit(0.8f, se::gui::GUIUnitType::Parent), se::gui::GUIUnitType::Auto));
		}
		backShape.setZIndex(10000);
		backShape.setPosition(se::gui::GUIVec2(glm::vec2(0.05f), se::gui::GUIUnitType::View));
		backShape.setAnchor(se::gui::GUIVec2(glm::vec2(0.0f), se::gui::GUIUnitType::Self));
		backShape.setSize(se::gui::GUIVec2(glm::vec2(0.2f, 0.1f), se::gui::GUIUnitType::View));
		backShape.setColor(se::Color(0.1f, 0.1f, 0.1f));
		backShape.onClick([&](se::gui::GUIElement&) { operatorGui.reset(new MonitorGui(context)); backShape.setVisible(false); });
		backShape.setVisible(false);
		setOperatorGui(IOperatorGui::OperatorGui::Monitor);
	}

	~Impl()
	{
		context.guiView.remove(backgroundShape);
	}

	void setOperatorGui(const IOperatorGui::OperatorGui _operatorGui)
	{
		backShape.setVisible(_operatorGui != IOperatorGui::OperatorGui::Monitor);
		switch (_operatorGui)
		{
		case IOperatorGui::OperatorGui::Monitor: operatorGui.reset(new MonitorGui(context)); break;
		case IOperatorGui::OperatorGui::Radar: operatorGui.reset(new RadarGui(context, playerCharacterServer, rootServer)); break;
		case IOperatorGui::OperatorGui::Mutator: operatorGui.reset(new MutatorGui(context, playerCharacterServer)); break;
		case IOperatorGui::OperatorGui::GeneSequencer: se_assert(false && "TODO"); break;
		case IOperatorGui::OperatorGui::MysteryGui: se_assert(false && "TODO"); break;
		}
	}

	void update()
	{
		if (const std::optional<IOperatorGui::OperatorGui> nextOperatorGui = operatorGui->update())
		{
			backShape.setVisible(*nextOperatorGui != IOperatorGui::OperatorGui::Monitor);
			switch (*nextOperatorGui)
			{
			case IOperatorGui::OperatorGui::Monitor: operatorGui.reset(new MonitorGui(context)); break;
			case IOperatorGui::OperatorGui::Radar: operatorGui.reset(new RadarGui(context, playerCharacterServer, rootServer)); break;
			case IOperatorGui::OperatorGui::Mutator: operatorGui.reset(new MutatorGui(context, playerCharacterServer)); break;
			case IOperatorGui::OperatorGui::GeneSequencer: se_assert(false && "TODO"); break;
			case IOperatorGui::OperatorGui::MysteryGui: se_assert(false && "TODO"); break;
			}
		}
	}

	ServerContext& context;
	PlayerCharacterServer& playerCharacterServer;
	RootServer& rootServer;
	se::gui::GUIShape backgroundShape;
	se::gui::GUIShape& backShape;
	std::unique_ptr<IOperatorGui> operatorGui;
};

OperatorHud::OperatorHud(ServerContext& _context, PlayerCharacterServer& _playerCharacterServer, RootServer& _rootServer)
	: impl(new Impl(_context, _playerCharacterServer, _rootServer))
{
}

OperatorHud::~OperatorHud()
{
	// ~Impl()
}

void OperatorHud::update()
{
	impl->update();
}

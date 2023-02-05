#include "stdafx.h"
#include "Base/Client/ClientHud.h"

#include "SpehsEngine/GUI/GuiView.h"
#include "SpehsEngine/GUI/GUIShape.h"
#include "SpehsEngine/GUI/GUIText.h"
#include "Base/UserSettingsWindow.h"
#pragma optimize("", off)

using namespace se::gui;
using namespace se::gui::unit_literals;


struct ClientHud::Impl
{
	Impl(DemoContext& _context, UserSettingsWindow& _userSettingsWindow)
		: context(_context)
		, userSettingsWindow(_userSettingsWindow)
	{
		context.guiView.add(settingsShape);
		settingsShape.setZIndex(-100);
		settingsShape.setPosition(0.98_view);
		settingsShape.setAnchor(1.0_self);
		settingsShape.setSize(0.07_vh);
		settingsShape.setColor(se::Color(0.2f, 0.22f, 0.2f));
		settingsShape.onClick([&](se::gui::GUIElement&) { userSettingsWindow.toggle(); });
		
		GUIText& text = settingsShape.addChild<se::gui::GUIText>();
		text.setZIndex(1000);
		text.insert("OPT");
		text.setPosition(se::gui::GUIVec2(glm::vec2(0.5f, 0.5f), se::gui::GUIUnitType::Parent));
		text.setAnchor(se::gui::GUIVec2(glm::vec2(0.5f, 0.5f), se::gui::GUIUnitType::Self));
		text.setSize(se::gui::GUIVec2(se::gui::GUIUnit(0.8f, se::gui::GUIUnitType::Parent), se::gui::GUIUnitType::Auto));
		text.setColor(se::Color(1.0f, 1.0f, 1.0f));
	}

	~Impl()
	{
		context.guiView.remove(settingsShape);
	}

	void update()
	{
	}

	DemoContext& context;
	UserSettingsWindow& userSettingsWindow;
	se::gui::GUIShape settingsShape;
};

ClientHud::ClientHud(DemoContext& _context, UserSettingsWindow& _userSettingsWindow)
	: impl(new Impl(_context, _userSettingsWindow))
{
}

ClientHud::~ClientHud()
{
	// ~Impl()
}

void ClientHud::update()
{
	impl->update();
}

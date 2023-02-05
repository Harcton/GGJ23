#include "stdafx.h"
#include "Base/GlobalHud.h"

#include "SpehsEngine/GUI/GuiView.h"
#include "SpehsEngine/GUI/GUIShape.h"
#include "SpehsEngine/GUI/GUIText.h"
#include "Base/UserSettingsWindow.h"
#pragma optimize("", off)

using namespace se::gui;
using namespace se::gui::unit_literals;


struct GlobalHud::Impl
{
	Impl(EngineContext& _context, UserSettingsWindow& _userSettingsWindow)
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
		{
			GUIText& text = settingsShape.addChild<se::gui::GUIText>();
			text.setZIndex(1000);
			text.insert("OPT");
			text.setPosition(se::gui::GUIVec2(glm::vec2(0.5f, 0.5f), se::gui::GUIUnitType::Parent));
			text.setAnchor(se::gui::GUIVec2(glm::vec2(0.5f, 0.5f), se::gui::GUIUnitType::Self));
			text.setSize(se::gui::GUIVec2(se::gui::GUIUnit(0.8f, se::gui::GUIUnitType::Parent), se::gui::GUIUnitType::Auto));
			text.setColor(se::Color(1.0f, 1.0f, 1.0f));
		}

		context.guiView.add(backShape);
		backShape.setZIndex(10000);
		backShape.setPosition(se::gui::GUIVec2(glm::vec2(0.05f, 0.95f), se::gui::GUIUnitType::View));
		backShape.setAnchor(se::gui::GUIVec2(glm::vec2(0.0f, 1.0f), se::gui::GUIUnitType::Self));
		backShape.setSize(se::gui::GUIVec2(glm::vec2(0.2f, 0.1f), se::gui::GUIUnitType::View));
		backShape.setColor(se::Color(0.1f, 0.1f, 0.1f));
		backShape.onClick([&](se::gui::GUIElement&) { backPressed = true; });
		{
			GUIText& text = backShape.addChild<se::gui::GUIText>();
			text.insert("BACK");
			text.setPosition(se::gui::GUIVec2(glm::vec2(0.5f, 0.5f), se::gui::GUIUnitType::Parent));
			text.setAnchor(se::gui::GUIVec2(glm::vec2(0.5f, 0.5f), se::gui::GUIUnitType::Self));
			text.setSize(se::gui::GUIVec2(se::gui::GUIUnit(0.8f, se::gui::GUIUnitType::Parent), se::gui::GUIUnitType::Auto));
			text.setZIndex(1);
		}
	}

	~Impl()
	{
		context.guiView.remove(backShape);
		context.guiView.remove(settingsShape);
	}

	void update()
	{
	}

	bool getBackPressed()
	{
		const bool result = backPressed;
		backPressed = false;
		return result;
	}

	void setBackEnabled(const bool _enabled)
	{
		backShape.setVisible(_enabled);
	}

	EngineContext& context;
	UserSettingsWindow& userSettingsWindow;
	se::gui::GUIShape settingsShape;
	se::gui::GUIShape backShape;
	bool backPressed = false;
};

GlobalHud::GlobalHud(EngineContext& _context, UserSettingsWindow& _userSettingsWindow)
	: impl(new Impl(_context, _userSettingsWindow))
{
}

GlobalHud::~GlobalHud()
{
	// ~Impl()
}

void GlobalHud::update()
{
	impl->update();
}

bool GlobalHud::getBackPressed()
{
	return impl->getBackPressed();
}

void GlobalHud::setBackEnabled(const bool _enabled)
{
	impl->setBackEnabled(_enabled);
}

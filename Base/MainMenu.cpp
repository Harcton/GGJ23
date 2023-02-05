#include "stdafx.h"
#include "Base/MainMenu.h"

#include "Base/Net/Packets.h"
#include "SpehsEngine/Core/DeltaTimeSystem.h"
#include "SpehsEngine/GUI/GUIStack.h"
#include "SpehsEngine/GUI/GUIShape.h"
#include "SpehsEngine/GUI/GUIView.h"
#include "SpehsEngine/GUI/GUIText.h"

#pragma optimize("", off)

using namespace se::gui;
using namespace se::gui::unit_literals;


struct MainMenu::Impl
{
	Impl(DemoContext& _context)
		: context(_context)
		, logo(rootElement.addChild<GUIShape>())
	{
		context.guiView.add(rootElement);
		rootElement.setPosition(GUIVec2(0.5_view));
		rootElement.setSize(0.0_view);

		const GUIVec2 textSize(0.8_parent, GUIUnitType::Auto);
		const GUIVec2 textAnchor(0.5_self);
		const GUIVec2 textPosition(0.5_parent);
		{
			GUIShape& shape = rootElement.addChild<GUIShape>();
			shape.setPosition(GUIVec2(glm::vec2(0.0f, 0.0f), GUIUnitType::Self));
			shape.setAnchor(GUIVec2(0.5_self, 0.5_self));
			shape.setSize(GUIVec2(0.5_vw, 0.2_vh));
			shape.onClick([this](GUIElement&) { mainMenuResult.emplace(MainMenuResult::Server); });
			shape.setColor(se::Color(0.2f, 0.2f, 0.22f));
			GUIText& text = shape.addChild<GUIText>();
			text.insert("Host game as the radio operator");
			text.setSize(textSize);
			text.setAnchor(textAnchor);
			text.setPosition(textPosition);
			text.setColor(se::Color(1.0f, 1.0f, 1.0f));
			text.setZIndex(1);
		}
		{
			GUIShape& shape = rootElement.addChild<GUIShape>();
			shape.setPosition(GUIVec2(glm::vec2(0.0f, 1.1f), GUIUnitType::Self));
			shape.setAnchor(GUIVec2(0.5_self, 0.5_self));
			shape.setSize(GUIVec2(0.5_vw, 0.2_vh));
			shape.onClick([this](GUIElement&) { mainMenuResult.emplace(MainMenuResult::Client); });
			shape.setColor(se::Color(0.2f, 0.2f, 0.22f));
			GUIText& text = shape.addChild<GUIText>();
			text.insert("Join game as a pilot");
			text.setSize(textSize);
			text.setAnchor(textAnchor);
			text.setPosition(textPosition);
			text.setZIndex(1);
		}

		logo.setPosition(GUIVec2(glm::vec2(-0.5f, -0.5f), GUIUnitType::Self));
		logo.setSize(1.0_view);
		logo.setTexture("roots-title.png");
		logo.setZIndex(-100);

		context.imguiBackend.connectToPreRenderSignal(scopedConnections.add(),
			[this]()
			{
				logoAnimationState += 0.1f * std::min(0.1f, context.deltaTimeSystem.deltaSeconds);
				logoAnimationState = std::min(1.0f, logoAnimationState);
				logo.setColor(se::mixColor(se::Color(1.0f, 1.0f, 1.0f), se::Color(1.0f, 0.01f, 0.05f), logoAnimationState));
			});
	}

	~Impl()
	{
		context.guiView.remove(rootElement);
	}

	DemoContext& context;
	GUIElement rootElement;
	GUIShape& logo;
	se::ScopedConnections scopedConnections;
	std::optional<MainMenuResult> mainMenuResult;
	float logoAnimationState = 0.0f;
};

MainMenu::MainMenu(DemoContext& _context)
	: impl(new Impl(_context))
{
}

MainMenu::~MainMenu()
{
	// ~Impl()
}

std::optional<MainMenuResult> MainMenu::getResult() const
{
	return impl->mainMenuResult;
}

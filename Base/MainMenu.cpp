#include "stdafx.h"
#include "Base/MainMenu.h"

#include "Base/Net/Packets.h"
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
	{
		context.guiView.add(guiStack);
		guiStack.setPosition(GUIVec2(0.5_view));
		guiStack.setAnchor(0.5_self);
		guiStack.setSize(GUIVec2(0.5_vh));

		const GUIVec2 textSize(0.8_parent, GUIUnitType::Auto);
		const GUIVec2 textAnchor(0.5_self);
		const GUIVec2 textPosition(0.5_parent);
		{
			GUIShape& shape = guiStack.addChild<GUIShape>();
			GUIText& text = shape.addChild<GUIText>();
			text.insert("Radar");
			text.setSize(textSize);
			text.setAnchor(textAnchor);
			text.setPosition(textPosition);
		}
	}

	~Impl()
	{
		context.guiView.remove(guiStack);
	}

	void update()
	{
	}

	DemoContext& context;
	GUIStack guiStack;
	se::ScopedConnections scopedConnections;
	std::optional<MainMenuResult> mainMenuResult;
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

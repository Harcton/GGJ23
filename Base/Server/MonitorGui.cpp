#include "stdafx.h"
#include "Base/Server/MonitorGui.h"

#include "Base/RenderTopLeftHelpTooltip.h"
#include "Base/Net/Packets.h"
#include "SpehsEngine/GUI/GUIShape.h"
#include "SpehsEngine/GUI/GUIView.h"
#include "SpehsEngine/GUI/GUIText.h"
#pragma optimize("", off)

using namespace se::gui;
using namespace se::gui::unit_literals;


struct MonitorGui::Impl
{
	Impl(ServerContext& _context)
		: context(_context)
	{
		context.guiView.add(rootShape);
		rootShape.setPosition(GUIVec2(0.5_view));
		rootShape.setAnchor(0.5_self);
		rootShape.setSize(GUIVec2(0.5_vh));
		rootShape.setColor(se::Color(0.1f, 0.1f, 0.1f));

		const GUIVec2 size(0.5_parent);
		const GUIVec2 textSize(0.8_parent, GUIUnitType::Auto);
		const GUIVec2 textAnchor(0.5_self);
		const GUIVec2 textPosition(0.5_parent);
		const se::Color color(0.3f, 0.3f, 0.3f);
		{
			GUIShape& shape = rootShape.addChild<GUIShape>();
			shape.setSize(size);
			shape.setColor(color);
			shape.setPosition(GUIVec2(glm::vec2(-0.1f, -0.1f), GUIUnitType::Self));
			shape.onClick([this](GUIElement&) {nextOperatorGui.emplace(OperatorGui::Radar); });
			shape.setTexture("radar-screen.png");
			GUIText&text = shape.addChild<GUIText>();
			text.insert("Radar");
			text.setSize(textSize);
			text.setAnchor(textAnchor);
			text.setPosition(textPosition);
		}
		{
			GUIShape& shape = rootShape.addChild<GUIShape>();
			shape.setSize(size);
			shape.setColor(color);
			shape.setPosition(GUIVec2(glm::vec2(-0.1f, 1.1f), GUIUnitType::Self));
			shape.onClick([this](GUIElement&) {nextOperatorGui.emplace(OperatorGui::Mutator); });
			shape.setTexture("loadout-screen.png");
			GUIText& text = shape.addChild<GUIText>();
			text.insert("Loadouts");
			text.setSize(textSize);
			text.setAnchor(textAnchor);
			text.setPosition(textPosition);
		}
		{
			GUIShape& shape = rootShape.addChild<GUIShape>();
			shape.setSize(size);
			shape.setColor(color);
			shape.setPosition(GUIVec2(glm::vec2(1.1f, -0.1f), GUIUnitType::Self));
			shape.onClick([this](GUIElement&) {nextOperatorGui.emplace(OperatorGui::GeneSequencer); });
			shape.setTexture("gene-sequencer.png");
			GUIText& text = shape.addChild<GUIText>();
			text.insert("Gene-sequencer");
			text.setSize(textSize);
			text.setAnchor(textAnchor);
			text.setPosition(textPosition);
		}
		{
			GUIShape& shape = rootShape.addChild<GUIShape>();
			shape.setSize(size);
			shape.setColor(color);
			shape.setPosition(GUIVec2(glm::vec2(1.1f, 1.1f), GUIUnitType::Self));
			shape.onClick([this](GUIElement&) {/*nextOperatorGui.emplace(OperatorGui::MysteryGui);*/ });
			shape.setTexture("bag-o-chips.png");
			GUIText& text = shape.addChild<GUIText>();
			text.insert("???");
			text.setSize(textSize);
			text.setAnchor(textAnchor);
			text.setPosition(textPosition);
			text.setColor(se::Color(0.4f, 0.4f, 0.4f));
		}

		context.imguiBackend.connectToPreRenderSignal(scopedConnections.add(),
			[this]()
			{
				renderTopLeftHelpTooltip("This is the main view of the radio-operator,\nwhere you can see the various devices you can interact with.\nClick on any of the screens to open a specific device menu.");
			});
	}

	~Impl()
	{
		context.guiView.remove(rootShape);
	}

	std::optional<OperatorGui> update()
	{
		return nextOperatorGui;
	}

	ServerContext& context;
	se::ScopedConnections scopedConnections;
	se::gui::GUIShape rootShape;
	std::optional<OperatorGui> nextOperatorGui;
};

MonitorGui::MonitorGui(ServerContext& _context)
	: impl(new Impl(_context))
{
}

MonitorGui::~MonitorGui()
{
	// ~Impl()
}

std::optional<IOperatorGui::OperatorGui> MonitorGui::update()
{
	return impl->update();
}

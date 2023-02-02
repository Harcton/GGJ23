#include "stdafx.h"
#include "Client/Playground.h"

#include "SpehsEngine/Debug/ImGfx.h"
#include "SpehsEngine/Graphics/Animator.h"
#include "SpehsEngine/Graphics/FontManager.h"
#include "SpehsEngine/Graphics/Lights.h"
#include "SpehsEngine/Graphics/Model.h"
#include "SpehsEngine/Graphics/ModelDataManager.h"
#include "SpehsEngine/Graphics/ShaderManager.h"
#include "SpehsEngine/Graphics/Shape.h"
#include "SpehsEngine/Graphics/Text.h"
#include "SpehsEngine/Graphics/TextureManager.h"
#include "SpehsEngine/GUI/GUIView.h"
#include "SpehsEngine/GUI/GUIShape.h"
#include "SpehsEngine/Input/EventSignaler.h"
#include "Base/DemoContext.h"
#include "Client/CameraController.h"
#include "Client/MaterialManager.h"
#include "Client/SoundPlayer.h"

using namespace se::graphics;
using namespace se::gui;
using namespace se::gui::unit_literals;


class Playground::Impl
{
public:
	Impl(DemoContext&);
	~Impl();
	void update();
private:

	DemoContext& context;
	se::ScopedConnections connections;
	glm::vec3 movement{};
	std::optional<SoundId> boingSoundId;
	SoundPlayer soundPlayer;
	MaterialManager materialManager;
	CameraController cameraController;
	Model frog;
	AmbientLight ambientLight;
	DirectionalLight sunLight;
	GUIElement guiRoot;
};

Playground::Playground(DemoContext& _context)
	: impl(std::make_unique<Impl>(_context))
{}
Playground::~Playground()
{}
void Playground::update()
{
	impl->update();
}

static constexpr int inputPriority = 12345;

Playground::Impl::~Impl()
{
	context.guiView.remove(guiRoot);
}
Playground::Impl::Impl(DemoContext& _context)
	: context(_context)
	, soundPlayer(_context)
	, materialManager(_context)
	, ambientLight(se::Color{}, 0.5f)
	, sunLight(se::Color{}, 0.5f, glm::vec3{1.0f, 1.0f, 1.0f})
	, cameraController(_context, inputPriority + 1)
{
	context.guiView.add(guiRoot);
	guiRoot.addChild<GUIShape>()
		.setColor(se::Color(se::DeepPink));

	context.scene.add(ambientLight);
	context.scene.add(sunLight);

	context.fontManager.create("playground_font", "Teko-Regular.ttf", FontSize{ 32 });

	auto bodyMat = materialManager.createMaterial(MaterialType::DemoFrog);
	bodyMat->setTexture(context.textureManager.create("body_tex", "body_tex.png"), PhongTextureType::Color);
	bodyMat->setTexture(context.textureManager.find("flat_normal"), PhongTextureType::Normal);
	bodyMat->setTexture(context.textureManager.find("roughness_1"), PhongTextureType::Roughness);
	auto eyeMat = materialManager.createMaterial(MaterialType::DemoFrog);
	eyeMat->setTexture(context.textureManager.create("eye_tex", "eye_tex.png"), PhongTextureType::Color);
	eyeMat->setTexture(context.textureManager.find("flat_normal"), PhongTextureType::Normal);
	eyeMat->setTexture(context.textureManager.find("roughness_0"), PhongTextureType::Roughness);

	frog.loadModelData(context.modelDataManager.create("frog", "demo_frog.gltf"));
	frog.setMaterial(eyeMat, 0);
	frog.setMaterial(bodyMat, 1);
	frog.getAnimator().start("Idle");
	context.scene.add(frog);

	context.eventSignaler.connectToKeyboardSignal(
		connections.add(),
		[this](const se::input::KeyboardEvent& _event)
		{
			if (_event.type != se::input::KeyboardEvent::Type::Hold)
				return false;
			switch (_event.key)
			{
				case se::input::Key::W: movement.z += 1.0f; return true;
				case se::input::Key::S: movement.z -= 1.0f; return true;
				case se::input::Key::A: movement.x += 1.0f; return true;
				case se::input::Key::D: movement.x -= 1.0f; return true;
			}
			return false;
		}, inputPriority);
	context.eventSignaler.connectToJoystickButtonSignal(
		connections.add(),
		[this](const se::input::JoystickButtonEvent& _event)
		{
			if (_event.type != se::input::JoystickButtonEvent::Type::Hold)
				return false;
			switch (_event.buttonIndex)
			{
				case 11: movement.z += 1.0f; return true;
				case 12: movement.z -= 1.0f; return true;
				case 13: movement.x += 1.0f; return true;
				case 14: movement.x -= 1.0f; return true;
			}
			return false;
		}, inputPriority);
	context.eventSignaler.connectToJoystickAxisSignal(
		connections.add(),
		[this](const se::input::JoystickAxisEvent& _event)
		{
			//se::log::info("JoystickAxisEvent");
			//se::log::info("	axisState: " + std::to_string(_event.axisState));
			//se::log::info("	axisIndex: " + std::to_string(_event.axisIndex));
			return false;
		}, inputPriority);

	soundPlayer.playMusic("chill.ogg", se::time::fromSeconds(2.0));
}
void Playground::Impl::update()
{
	cameraController.update();

	const bool moving = glm::length(movement) != 0.0f;
	if (moving)
	{
		movement = glm::normalize(movement);

		constexpr float speed = 10.0f;
		frog.setPosition(frog.getPosition() + movement * speed * context.deltaTimeSystem.deltaSeconds);
		frog.setRotation(glm::lookAt(glm::vec3{}, { movement.z, movement.y, movement.x }, glm::vec3{ 0.0f, 1.0f, 0.0f }));
	}

	if (moving)
	{
		if (!boingSoundId.has_value())
		{
			boingSoundId = soundPlayer.playSound("boing.ogg", frog.getPosition());
			soundPlayer.setSoundLooping(boingSoundId.value(), true);
		}
		else
		{
			soundPlayer.setSoundPosition(boingSoundId.value(), frog.getPosition());
		}
	}
	else if (!moving && boingSoundId.has_value())
	{
		soundPlayer.stopSound(boingSoundId.value());
		boingSoundId.reset();
	}

	{
		const std::string currentAnimation = moving ? "Boing" : "Idle";
		if (!frog.getAnimator().isPlaying(currentAnimation))
		{
			constexpr auto fade = se::time::fromSeconds(0.2f);
			frog.getAnimator().stopAll(fade);
			frog.getAnimator().start(currentAnimation, fade);
			frog.getAnimator().setSpeed("Boing", 2.0f);
		}
	}
	movement = {};

	ImGfx::shape(ShapeType::Square)
		.color(se::Color{ se::LightSlateGray })
		.scale(glm::vec3{ 1000.0f });
	ImGfx::text("Testing... 123...", "playground_font")
		.position(glm::vec3{ 0.0f, 0.1f, 0.0f })
		.scale(glm::vec3{ 0.1f });

	if (ImGui::Begin("Playground"))
	{
		float masterVolume = soundPlayer.getMasterVolume();
		if (ImGui::SliderFloat("Master Volume", &masterVolume, 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp))
		{
			soundPlayer.setMasterVolume(masterVolume);
		}
		float musicVolume = soundPlayer.getMusicVolume();
		if (ImGui::SliderFloat("Music Volume", &musicVolume, 0.0f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp))
		{
			soundPlayer.setMusicVolume(musicVolume);
		}
	} ImGui::End();
}

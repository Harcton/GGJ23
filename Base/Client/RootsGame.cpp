#include "stdafx.h"
#include "Base/Client/RootsGame.h"

#include "SpehsEngine/Debug/ImGfx.h"
#include "SpehsEngine/Graphics/Animator.h"
#include "SpehsEngine/Graphics/FontManager.h"
#include "SpehsEngine/Graphics/Lights.h"
#include "SpehsEngine/Graphics/Model.h"
#include "SpehsEngine/Graphics/View.h"
#include "SpehsEngine/Graphics/Renderer.h"
#include "SpehsEngine/Graphics/Window.h"
#include "SpehsEngine/Graphics/Camera.h"
#include "SpehsEngine/Graphics/ModelDataManager.h"
#include "SpehsEngine/Graphics/InstanceBuffer.h"
#include "SpehsEngine/Graphics/ShaderManager.h"
#include "SpehsEngine/Graphics/Shape.h"
#include "SpehsEngine/Graphics/Text.h"
#include "SpehsEngine/Graphics/TextureManager.h"
#include "SpehsEngine/GUI/GUIView.h"
#include "SpehsEngine/GUI/GUIShape.h"
#include "SpehsEngine/GUI/GUIText.h"
#include "SpehsEngine/Input/EventSignaler.h"
#include "Base/ClientUtility/CameraController.h"
#include "Base/ClientUtility/MaterialManager.h"
#include "Base/ClientUtility/SoundPlayer.h"
#include "Base/Client/EvilRootManager.h"
#include "Base/Client/PlayerCharacter.h"
#include "Base/Client/BulletManager.h"

using namespace se::graphics;
using namespace se::gui;
using namespace se::gui::unit_literals;



struct RootsGame::Impl
{
	Impl(ClientContext& _context);
	~Impl()
	{
		context.guiView.remove(winScreenBg);
		context.guiView.remove(missionText);
	}
	bool update();
	void showEndUI(bool win)
	{
		missionText.setVisible(false);

		winScreenBg.setVisible(true);
		winScreenBg.setWidth(1_view);
		winScreenBg.setHeight(0.33_view);
		winScreenBg.setColor(se::Color(se::Black));
		winScreenBg.setAlignment(VerticalAlignment::Center, HorizontalAlignment::Center);
		winScreenBg.setAnchor(VerticalAnchor::Center, HorizontalAnchor::Center);

		GUIText& text = winScreenBg.addChild<GUIText>();
		text.insert(win ? "VICTORY" : "YA'LL DEAD");
		text.setColor(win ? se::Color(se::Gold) : se::Color(se::DarkRed));
		text.setWidth(0.5_pw);
		text.setFont("teko");
		text.setAlignment(VerticalAlignment::Center, HorizontalAlignment::Center);
		text.setAnchor(VerticalAnchor::Center, HorizontalAnchor::Center);
	}

	ClientContext& context;
	View observerView;
	Window observerWindow;
	Camera observerCamera;

	se::ScopedConnections connections;
	BulletManager bulletManager;
	PlayerCharacter player;
	EvilRootManager rootManager;

	GUIShape winScreenBg;
	GUIText missionText;

	Shape ground;
	Model wall;
	Model wallRoots;
	Model coreAntenna;
	Model coreRingA;
	Model coreRingB;
	Model coreStructure;

	AmbientLight ambientLight;
	DirectionalLight sunLight;
	PointLight coreLight;
	bool panicTime = false;
	se::time::Time started = se::time::now();
	bool exitGame = false;
};
RootsGame::RootsGame(ClientContext& _context)
	: impl(std::make_unique<Impl>(_context)) {}
RootsGame::~RootsGame()
{}
bool RootsGame::update()
{
	return impl->update();
}


RootsGame::Impl::Impl(ClientContext& _context)
	: context(_context)
	, ambientLight(se::Color{}, 0.6f)
	, sunLight(se::Color{}, 0.8f, glm::vec3{ 4.0f, 5.0f, 1.0f })
	, bulletManager(_context, constants::worldSize)
	, rootManager(_context, bulletManager, constants::worldSize)
	, player(_context, bulletManager)
	, observerView(_context.scene, observerCamera)
{
	context.guiView.add(winScreenBg);
	winScreenBg.setVisible(false);
	//winScreenBg.onClick([this](auto&){ exitGame = true; });
	context.fontManager.create("teko", "Teko-Regular.ttf", FontSize{ 72 });

	context.guiView.add(missionText);
	missionText.setY(8_px);
	missionText.setHeight(0.035_vh);
	missionText.setHorizontalAnchor(HorizontalAnchor::Center);
	missionText.setHorizontalAlignment(HorizontalAlignment::Center);
	missionText.setFont("teko");

	context.packetman.registerReceiveHandler<GameEndPacket>(
		PacketType::GameEnd, connections.add(),
		[this](GameEndPacket& _packet, const bool _reliable)
		{
			showEndUI(_packet.win);
		});

	context.scene.add(ambientLight);
	context.scene.add(sunLight);

	observerWindow.setName("Debug Overview");
	observerWindow.setX(50);
	observerWindow.setY(50);
	observerWindow.add(observerView);
	observerCamera.setPosition(glm::vec3{ 0.0f, 500.0f, -150.0f });
	observerCamera.setDirection(glm::normalize(-observerCamera.getPosition()));
	context.userSettings.connectToEnableDebugOverviewWindowChangedSignal(connections.add(),
		[this](const bool&, const bool& newValue)
		{
			if (newValue)
			{
				context.renderer.add(observerWindow);
			}
			else
			{
				context.renderer.remove(observerWindow);
			}
		});
	if (context.userSettings.getEnableDebugOverviewWindow())
	{
		context.renderer.add(observerWindow);
	}

	{
		auto material = context.materialManager.createMaterial(DefaultMaterialType::Phong);
		material->setTexture(context.textureManager.create("ground_green_dif.png", "ground_green_dif.png"), PhongTextureType::Color);
		material->setTexture(context.textureManager.create("ground_normal.png", "ground_normal.png"), PhongTextureType::Normal);

		ShapeParameters params = context.materialManager.getDefaultShapeParams();
		params.resolution = 100;
		params.uvScale = 7.5f;

		ground.generate(ShapeType::Circle, params, &context.shapeGenerator);
		ground.setMaterial(material);
		ground.setColor(se::Color(0.6f, 0.6f, 0.6f));
		ground.setScale(glm::vec3{ constants::worldSize, 1.0f, constants::worldSize });
		context.scene.add(ground);
	}
	{
		constexpr glm::vec3 actualCoreSize{ 1.3f };

		auto mat = context.materialManager.getDefaultMaterial();
		mat->setTexture(context.textureManager.create("color_gradient_dif.png", "color_gradient_dif.png"), PhongTextureType::Color);

		coreStructure.loadModelData(context.modelDataManager.create("core_structure", "Base_Structure.fbx"));
		coreStructure.setMaterial(mat);
		coreStructure.setScale(actualCoreSize);
		context.scene.add(coreStructure);

		coreRingA.loadModelData(context.modelDataManager.create("core_ring_a", "Base_Ring_A.fbx"));
		coreRingA.setMaterial(mat);
		coreRingA.setScale(actualCoreSize);
		context.scene.add(coreRingA);

		coreRingB.loadModelData(context.modelDataManager.create("core_ring_b", "Base_Ring_B.fbx"));
		coreRingB.setMaterial(mat);
		coreRingB.setScale(actualCoreSize);
		context.scene.add(coreRingB);

		coreAntenna.loadModelData(context.modelDataManager.create("core_antenna", "Base_Antenna.fbx"));
		coreAntenna.setMaterial(mat);
		coreAntenna.setScale(actualCoreSize);
		context.scene.add(coreAntenna);

		coreLight.setColor(se::Color(se::DarkRed));
		coreLight.setPosition(coreStructure.getPosition() + glm::vec3{ 0.0f, 20.0f, 0.0f });
		coreLight.setIntensity(1.0f);
		coreLight.setRadius(1.0f, 100.0f);
	}
	{
		auto mat = context.materialManager.getDefaultMaterial();
		mat->setTexture(context.textureManager.create("root_wall.png", "root_wall.png"), PhongTextureType::Color);
		mat->setTexture(context.textureManager.create("root_wall_normal.png", "root_wall_normal.png"), PhongTextureType::Normal);

		wall.loadModelData(context.modelDataManager.create("wall", "wall_whole_v1.fbx"));
		wall.setMaterial(mat);
		wall.disableRenderFlags(RenderFlag::CullBackFace);
		wall.setScale(glm::vec3{ constants::worldSize });
		context.scene.add(wall);

		wallRoots.loadModelData(context.modelDataManager.create("wall_roots", "Roots_Wall_2.fbx"));
		wallRoots.setMaterial(context.materialManager.getDefaultMaterial());
		wallRoots.disableRenderFlags(RenderFlag::CullBackFace);
		wallRoots.setScale(glm::vec3{ constants::worldSize });
		wallRoots.setColor(constants::rootColor);
		context.scene.add(wallRoots);
	}

	context.soundPlayer.playMusic("GunFightTheme_01.ogg", se::time::fromSeconds(1.0f));
}
bool RootsGame::Impl::update()
{
	const se::time::Time timeLeft = constants::gameWinTime - se::time::timeSince(started);
	const float gameProgress = 1.0f - (timeLeft.asSeconds() / constants::gameWinTime.asSeconds());
	missionText.clear();
	missionText.insert("Mission: Defend Base! (" + std::to_string((int)timeLeft.asSeconds()) + "s)");

	rootManager.update();
	player.update();
	bulletManager.update();

	coreRingA.setRotation(
		glm::rotate(
			coreRingA.getRotation(),
			se::HALF_PI<float> * gameProgress * context.deltaTimeSystem.deltaSeconds,
			glm::vec3{ 0.0f, 1.0f, 0.0f }));

	coreRingB.setRotation(
		glm::rotate(
			coreRingB.getRotation(),
			-se::HALF_PI<float> * 1.2f * gameProgress * context.deltaTimeSystem.deltaSeconds,
			glm::vec3{ 0.0f, 1.0f, 0.0f }));

	if (!panicTime && se::time::timeSince(started) > se::time::fromSeconds(120.0))
	{
		panicTime = true;
		context.scene.add(coreLight);
	}
	coreLight.setIntensity(glm::mix(coreLight.getIntensity(), se::rng::random(0.0f, 1.0f), 20.0f * context.deltaTimeSystem.deltaSeconds));

	return !exitGame; // TODO: return false to go back to main menu
}

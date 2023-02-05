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
#include "SpehsEngine/Graphics/ShaderManager.h"
#include "SpehsEngine/Graphics/Shape.h"
#include "SpehsEngine/Graphics/Text.h"
#include "SpehsEngine/Graphics/TextureManager.h"
#include "SpehsEngine/GUI/GUIView.h"
#include "SpehsEngine/GUI/GUIShape.h"
#include "SpehsEngine/Input/EventSignaler.h"
#include "Base/ClientUtility/CameraController.h"
#include "Base/ClientUtility/MaterialManager.h"
#include "Base/ClientUtility/SoundPlayer.h"
#include "Base/Client/EvilRootManager.h"
#include "Base/Client/PlayerCharacter.h"
#include "Base/Client/BulletManager.h"

using namespace se::graphics;



struct RootsGame::Impl
{
	Impl(ClientContext& _context);
	~Impl() = default;
	bool update();

	ClientContext& context;
	View observerView;
	Window observerWindow;
	Camera observerCamera;

	se::ScopedConnections connections;
	BulletManager bulletManager;
	PlayerCharacter player;
	EvilRootManager rootManager;

	Shape ground;
	Model core;

	AmbientLight ambientLight;
	DirectionalLight sunLight;
	PointLight coreLight;
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
	, ambientLight(se::Color{}, 0.5f)
	, sunLight(se::Color{}, 0.75f, glm::vec3{ 2.0f, 5.0f, 1.0f })
	, bulletManager(_context, constants::worldSize)
	, rootManager(_context, bulletManager, constants::worldSize)
	, player(_context, bulletManager)
	, observerView(_context.scene, observerCamera)
{
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
		material->setTexture(context.textureManager.create("ground.png", "ground.png"), PhongTextureType::Color);
		material->setTexture(context.textureManager.find("flat_normal"), PhongTextureType::Normal);

		ShapeParameters params = context.materialManager.getDefaultShapeParams();
		params.resolution = 100;
		params.uvScale = 7.5f;

		ground.generate(ShapeType::Circle, params, &context.shapeGenerator);
		ground.setMaterial(material);
		ground.setScale(glm::vec3{ constants::worldSize, 1.0f, constants::worldSize });
		context.scene.add(ground);
	}
	{
		core.loadModelData(context.modelDataManager.create("core", "Base_Structure.fbx"));
		core.setMaterial(context.materialManager.getDefaultMaterial());
		core.setScale(glm::vec3{ 1.3f });
		//core.setScale(glm::vec3{ constants::coreSize });
		context.scene.add(core);

		coreLight.setColor(se::Color(se::DarkRed));
		coreLight.setPosition(core.getPosition() + glm::vec3{ 0.0f, 20.0f, 0.0f });
		coreLight.setIntensity(0.8f);
		coreLight.setRadius(1.0f, 100.0f);
		context.scene.add(coreLight);
	}

	context.soundPlayer.playMusic("GunFightTheme_01.ogg", se::time::fromSeconds(1.0f));
}
bool RootsGame::Impl::update()
{
	rootManager.update();
	player.update();
	bulletManager.update();

	coreLight.setIntensity(glm::mix(coreLight.getIntensity(), se::rng::random(0.0f, 1.0f), 20.0f * context.deltaTimeSystem.deltaSeconds));

	return true; // TODO: return false to go back to main menu
}

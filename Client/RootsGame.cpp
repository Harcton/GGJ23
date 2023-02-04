#include "stdafx.h"
#include "Client/RootsGame.h"

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
#include "Client/EvilRootManager.h"
#include "Client/PlayerCharacter.h"
#include "Client/BulletManager.h"

using namespace se::graphics;



struct RootsGame::Impl
{
	Impl(ClientContext& _context);
	~Impl() = default;
	void update();

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
};
RootsGame::RootsGame(ClientContext& _context)
	: impl(std::make_unique<Impl>(_context)) {}
RootsGame::~RootsGame()
{}
void RootsGame::update()
{
	impl->update();
}


RootsGame::Impl::Impl(ClientContext& _context)
	: context(_context)
	, ambientLight(se::Color{}, 0.5f)
	, sunLight(se::Color{}, 0.5f, glm::vec3{ 1.0f, 5.0f, 1.0f })
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
		params.uvScale = 10.0f;

		ground.generate(ShapeType::Circle, params, &context.shapeGenerator);
		ground.setMaterial(material);
		ground.setScale(glm::vec3{ constants::worldSize, 1.0f, constants::worldSize });
		context.scene.add(ground);
	}
	{
		core.loadModelData(context.modelDataManager.create("core", "Base_Structure.fbx"));
		core.setMaterial(context.materialManager.getDefaultMaterial());
		//core.setScale(glm::vec3{ constants::coreSize });
		context.scene.add(core);
	}

	context.soundPlayer.playMusic("main_theme_root_bgm.ogg", se::time::fromSeconds(2.0f));
}
void RootsGame::Impl::update()
{
	rootManager.update();
	player.update();
	bulletManager.update();
}

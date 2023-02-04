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
	Shape core;

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


constexpr float worldSize = 500.0f;

RootsGame::Impl::Impl(ClientContext& _context)
	: context(_context)
	, ambientLight(se::Color{}, 1.0f)
	, sunLight(se::Color{}, 1.0f, glm::vec3{ 1.0f, 2.0f, 1.0f })
	, bulletManager(_context, worldSize)
	, rootManager(_context, bulletManager, worldSize)
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
		ShapeParameters params;
		params.generateNormals = true;
		params.generateTangents = true;
		params.resolution = 100;
		//params.uvScale = 1.0f / worldSize;

		auto material = context.materialManager.createMaterial(DefaultMaterialType::Phong);
		material->setTexture(context.textureManager.create("ground.png", "ground.png"), PhongTextureType::Color);
		material->setTexture(context.textureManager.find("flat_normal"), PhongTextureType::Normal);
		//material->setTexture(context.textureManager.find("roughness_1"), PhongTextureType::Roughness);

		ground.generate(ShapeType::Circle, params, &context.shapeGenerator);
		ground.setMaterial(material);
		ground.setScale(glm::vec3{ worldSize });
		context.scene.add(ground);
	}
	{
		auto material = context.materialManager.createMaterial(DefaultMaterialType::Phong);
		material->setTexture(context.textureManager.find("white_color"), PhongTextureType::Color);
		material->setTexture(context.textureManager.find("flat_normal"), PhongTextureType::Normal);

		core.generate(ShapeType::Sphere, ShapeParameters{}, &context.shapeGenerator);
		core.setMaterial(material);
		core.setScale(glm::vec3{ 10.0f });
		context.scene.add(core);
	}
}
void RootsGame::Impl::update()
{
	rootManager.update();
	player.update();
	bulletManager.update();
}

#include "stdafx.h"
#include "Base/DemoContext.h"

#include "Base/DefaultResourcePathFinders.h"
#include "SpehsEngine/Core/ScopeTimer.h"
#include "SpehsEngine/Debug/ImGfx.h"

using namespace se::graphics;


DemoContext::DemoContext(const std::string_view _windowName)
	: mainWindow(true)
	, renderer(mainWindow, RendererFlag::VSync | RendererFlag::MSAA4, RendererBackend::Direct3D11)
	, view(scene, camera)
	, imguiBackend(eventSignaler, 0, renderer)
	, imGraphics(view, shaderManager, textureManager, fontManager, modelDataManager, shapeGenerator)
{
	se::time::ScopeTimer initTimer;

	///////////////
	// Graphics

	// Main window
	mainWindow.setName(_windowName);

	// Main view
	mainWindow.add(view);

	// Camera settings
	camera.setFar(50000.0f);

	// Resource Management
	auto graphicsResourceLoader = se::graphics::makeResourceLoader(8);
	shaderManager.setResourcePathFinder(std::make_shared<ShaderPathFinder>());
	shaderManager.setResourceLoader(graphicsResourceLoader);
	shaderManager.createDefaultShaders();
	textureManager.setResourcePathFinder(std::make_shared<TexturePathFinder>());
	textureManager.setResourceLoader(graphicsResourceLoader);
	fontManager.setResourcePathFinder(std::make_shared<FontPathFinder>());
	fontManager.setResourceLoader(graphicsResourceLoader);
	fontManager.createDefaultFonts();
	modelDataManager.setResourcePathFinder(std::make_shared<ModelPathFinder>());
	modelDataManager.setResourceLoader(graphicsResourceLoader);

	imGraphics.init();
	ImGfx::init(imGraphics);


	///////////////
	// Audio

	auto audioPathFinder = std::make_shared<AudioPathFinder>();
	auto audioResourceLoader = se::audio::makeResourceLoader(4);
	audioManager.setResourceLoader(audioResourceLoader);
	audioManager.setResourcePathFinder(audioPathFinder);

	se::log::info("DemoContext init time: " + std::to_string(initTimer.get().asSeconds()) + " seconds", se::log::GREEN);
}
DemoContext::~DemoContext()
{
	ImGfx::deinit();
}
void DemoContext::reset()
{
	shaderManager.purgeUnused();
	textureManager.purgeUnused();
	fontManager.purgeUnused();
	modelDataManager.purgeUnused();
	shapeGenerator.clear();
	audioManager.purgeUnused();

	imGraphics.cleanup();

	inputManager.ignoreQuitRequest();
	mainWindow.ignoreQuitRequest();
}
bool DemoContext::update()
{
	deltaTimeSystem.update();

	shaderManager.update();
	textureManager.update();
	fontManager.update();
	modelDataManager.update();

	audioEngine.setListenerDirection(camera.getDirection());
	audioEngine.setListenerPosition(camera.getPosition());
	audioEngine.setListenerUp(camera.getUp());
	audioEngine.update();

	eventCatcher.pollEvents();
	inputManager.update(eventCatcher);
	eventSignaler.signalEvents(eventCatcher);

	return !inputManager.isQuitRequested() && !mainWindow.isQuitRequested();
}
void DemoContext::render()
{
	imguiBackend.render();
	renderer.render();
	imGraphics.endFrame();
}
void DemoContext::showWindowDefault()
{
	mainWindow.setBorderless(false);
	mainWindow.setWidth(1600);
	mainWindow.setHeight(900);
	mainWindow.setCenteredX();
	mainWindow.setCenteredY();
	mainWindow.show();
}
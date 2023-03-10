#include "stdafx.h"
#include "Base/DemoContextState.h"

#include "Base/DefaultResourcePathFinders.h"
#include "Base/MutationDatabase.h"
#include "SpehsEngine/Audio/AudioEngine.h"
#include "SpehsEngine/Audio/AudioManager.h"
#include "SpehsEngine/Core/DeltaTimeSystem.h"
#include "SpehsEngine/Core/ScopeTimer.h"
#include "SpehsEngine/Debug/ImGfx.h"
#include "SpehsEngine/Debug/ImmediateModeGraphics.h"
#include "SpehsEngine/Graphics/Camera.h"
#include "SpehsEngine/Graphics/FontManager.h"
#include "SpehsEngine/Graphics/ModelDataManager.h"
#include "SpehsEngine/Graphics/Renderer.h"
#include "SpehsEngine/Graphics/Scene.h"
#include "SpehsEngine/Graphics/ShaderManager.h"
#include "SpehsEngine/Graphics/ShapeGenerator.h"
#include "SpehsEngine/Graphics/TextureManager.h"
#include "SpehsEngine/Graphics/View.h"
#include "SpehsEngine/Graphics/Window.h"
#include "SpehsEngine/GUI/GUIView.h"
#include "SpehsEngine/ImGui/Utility/BackendWrapper.h"
#include "SpehsEngine/Input/EventCatcher.h"
#include "SpehsEngine/Input/EventSignaler.h"
#include "SpehsEngine/Input/InputManager.h"
#include "SpehsEngine/Net/ConnectionManager2.h"
#include "Base/ClientUtility/MaterialManager.h"
#include "Base/ClientUtility/SoundPlayer.h"
#include "Base/UserSettings.h"
#include "Base/UserSettingsWindow.h"
#include "Base/GlobalHud.h"
#pragma optimize("", off)

struct DemoContextState::Impl
{

	Impl(const std::string_view _windowName, const std::string_view _processFilepath)
		: processFilepath(_processFilepath)
		, mainWindow(true)
		, renderer(mainWindow, se::graphics::RendererFlag::VSync /*| se::graphics::RendererFlag::MSAA4*/, se::graphics::RendererBackend::Direct3D11)
		, view(scene, camera)
		, guiView(shaderManager, textureManager, fontManager, eventSignaler, 9001)
		, imguiBackend(eventSignaler, 0, renderer)
		, imGraphics(view, shaderManager, textureManager, fontManager, modelDataManager, shapeGenerator)
		, connectionManager(_windowName)
		, engineContext
		{
			processFilepath,
			deltaTimeSystem,
			eventCatcher,
			inputManager,
			eventSignaler,
			mainWindow,
			renderer,
			scene,
			camera,
			view,
			shaderManager,
			textureManager,
			fontManager,
			modelDataManager,
			shapeGenerator,
			imGraphics,
			guiView,
			imguiBackend,
			audioEngine,
			audioManager,
			connectionManager,
		}
		, userSettings(_windowName)
		, materialManager(shaderManager, textureManager)
		, soundPlayer(audioManager, audioEngine)
		, userSettingsWindow(engineContext, userSettings)
		, globalHud(engineContext, userSettingsWindow)
		, demoContext
		{
			engineContext,
			mutationDatabase,
			userSettings,
			materialManager,
			soundPlayer,
			globalHud,
		}
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

		materialManager.init();
		soundPlayer.init();


		///////////////
		// GUI

		mainWindow.add(guiView.getView());

		se::graphics::TextureInput textureInput;
		textureInput.width = 1;
		textureInput.height = 1;
		textureInput.data = { 255, 255, 255, 255 };
		constexpr se::graphics::TextureModes pointTextureModes
			{ se::graphics::TextureWrappingMode::Repeat, se::graphics::TextureWrappingMode::Repeat, se::graphics::TextureWrappingMode::Repeat,
			  se::graphics::TextureSamplingMode::Linear,  se::graphics::TextureSamplingMode::Linear,  se::graphics::TextureSamplingMode::Linear };
		textureManager.create("data/assets/texture/gene-sequencer.png", textureInput, pointTextureModes);

		///////////////
		// Audio

		auto audioPathFinder = std::make_shared<AudioPathFinder>();
		auto audioResourceLoader = se::audio::makeResourceLoader(4);
		audioManager.setResourceLoader(audioResourceLoader);
		audioManager.setResourcePathFinder(audioPathFinder);

		// User settings
		userSettings.connectToResolutionChangedSignal(scopedConnections.add(),
			[this](const glm::ivec2 &, const glm::ivec2 &newValue)
			{
				mainWindow.setWidth(newValue.x);
				mainWindow.setHeight(newValue.y);
			}, true);
		userSettings.connectToFullscreenChangedSignal(scopedConnections.add(),
			[this](const bool&, const bool& newValue)
			{
				mainWindow.setBorderless(newValue);
				if (newValue)
				{
					mainWindow.setX(0);
					mainWindow.setY(0);
				}
			}, true);
		userSettings.connectToVolumeMasterChangedSignal(scopedConnections.add(), [this](const float&, const float& newValue){ soundPlayer.setMasterVolume(newValue); }, true);
		userSettings.connectToVolumeMusicChangedSignal(scopedConnections.add(), [this](const float&, const float& newValue){ soundPlayer.setMusicVolume(newValue); }, true);
		userSettings.connectToVolumeSFXChangedSignal(scopedConnections.add(), [this](const float&, const float& newValue){ soundPlayer.setSfxVolume(newValue); }, true);
		mainWindow.setBorderless(false);
		mainWindow.setCenteredX();
		mainWindow.setCenteredY();
		mainWindow.show();

		se::log::info("DemoContext init time: " + std::to_string(initTimer.get().asSeconds()) + " seconds", se::log::GREEN);
	}

	~Impl()
	{
		ImGfx::deinit();
	}

	void reset()
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

	bool update()
	{
		deltaTimeSystem.update();
		userSettings.update();
		connectionManager.update();
		globalHud.update();

		shaderManager.update();
		textureManager.update();
		fontManager.update();
		modelDataManager.update();

		audioEngine.setListenerDirection(camera.getDirection());
		audioEngine.setListenerPosition(camera.getPosition());
		audioEngine.setListenerUp(camera.getUp());
		audioEngine.update();

		soundPlayer.update();

		eventCatcher.pollEvents();
		inputManager.update(eventCatcher);
		eventSignaler.signalEvents(eventCatcher);

		return !inputManager.isQuitRequested() && !mainWindow.isQuitRequested();
	}

	void render()
	{
		imguiBackend.render();
		renderer.render();
		imGraphics.endFrame();
	}

	DemoContext getDemoContext()
	{
		return demoContext;
	}

	const std::string processFilepath;

	se::time::DeltaTimeSystem deltaTimeSystem;

	// Input
	se::input::EventCatcher eventCatcher;
	se::input::InputManager inputManager;
	se::input::EventSignaler eventSignaler;

	// Graphics
	se::graphics::Window mainWindow;
	se::graphics::Renderer renderer;
	se::graphics::Scene scene;
	se::graphics::Camera camera;
	se::graphics::View view;
	se::graphics::ShaderManager shaderManager;
	se::graphics::TextureManager textureManager;
	se::graphics::FontManager fontManager;
	se::graphics::ModelDataManager modelDataManager;
	se::graphics::ShapeGenerator shapeGenerator;
	se::debug::ImmediateModeGraphics imGraphics;

	se::gui::GUIView guiView;

	// ImGui
	se::imgui::BackendWrapper imguiBackend;

	// Audio
	se::audio::AudioEngine audioEngine;
	se::audio::AudioManager audioManager;

	se::net::ConnectionManager2 connectionManager;

	EngineContext engineContext;

	// GAME:


	UserSettings userSettings;

	MutationDatabase mutationDatabase;

	MaterialManager materialManager;
	SoundPlayer soundPlayer;
	GlobalHud globalHud;
	DemoContext demoContext;

	UserSettingsWindow userSettingsWindow;
	se::ScopedConnections scopedConnections;
};

DemoContextState::DemoContextState(const std::string_view _windowName, const std::string_view _processFilepath)
	: impl(new Impl(_windowName, _processFilepath))
{
}

DemoContextState::~DemoContextState()
{
	// ~Impl()
}

void DemoContextState::reset()
{
	impl->reset();
}

bool DemoContextState::update()
{
	return impl->update();
}

void DemoContextState::render()
{
	impl->render();
}

DemoContext DemoContextState::getDemoContext()
{
	return impl->getDemoContext();
}

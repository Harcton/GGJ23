#pragma once

#include "SpehsEngine/Audio/AudioEngine.h"
#include "SpehsEngine/Audio/AudioManager.h"
#include "SpehsEngine/Core/DeltaTimeSystem.h"
#include "SpehsEngine/Debug/ImmediateModeGraphics.h"
#include "SpehsEngine/Graphics/Camera.h"
#include "SpehsEngine/Graphics/FontManager.h"
#include "SpehsEngine/Graphics/ModelDataManager.h"
#include "SpehsEngine/Graphics/Renderer.h"
#include "SpehsEngine/Graphics/Scene.h"
#include "SpehsEngine/Graphics/ShaderManager.h"
#include "SpehsEngine/Graphics/TextureManager.h"
#include "SpehsEngine/Graphics/ShapeGenerator.h"
#include "SpehsEngine/Graphics/View.h"
#include "SpehsEngine/Graphics/Window.h"
#include "SpehsEngine/ImGui/Utility/BackendWrapper.h"
#include "SpehsEngine/Input/EventCatcher.h"
#include "SpehsEngine/Input/EventSignaler.h"
#include "SpehsEngine/Input/InputManager.h"


struct DemoContext
{
	DemoContext(const std::string_view _windowName);
	~DemoContext();

	void reset();
	bool update();
	void render();
	void showWindowDefault();


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

	// ImGui
	se::imgui::BackendWrapper imguiBackend;

	// Audio
	se::audio::AudioEngine audioEngine;
	se::audio::AudioManager audioManager;
};
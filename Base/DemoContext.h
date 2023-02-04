#pragma once

namespace se
{
	namespace debug
	{
		class ImmediateModeGraphics;
	}
	namespace imgui
	{
		class BackendWrapper;
	}
	namespace time
	{
		class DeltaTimeSystem;
	}
	namespace audio
	{
		class AudioEngine;
		class AudioManager;
	}
	namespace input
	{
		class EventCatcher;
		class InputManager;
		class EventSignaler;
	}
	namespace graphics
	{
		class Window;
		class Renderer;
		class Scene;
		class Camera;
		class View;
		class ShaderManager;
		class TextureManager;
		class FontManager;
		class ModelDataManager;
		class ShapeGenerator;
	}
	namespace gui
	{
		class GUIView;
	}
}

struct MutationDatabase;
class MaterialManager;
class UserSettings;
class SoundPlayer;


struct DemoContext
{
	se::time::DeltaTimeSystem &deltaTimeSystem;

	// Input
	se::input::EventCatcher &eventCatcher;
	se::input::InputManager &inputManager;
	se::input::EventSignaler &eventSignaler;

	// Graphics
	se::graphics::Window &mainWindow;
	se::graphics::Renderer &renderer;
	se::graphics::Scene &scene;
	se::graphics::Camera &camera;
	se::graphics::View &view;
	se::graphics::ShaderManager &shaderManager;
	se::graphics::TextureManager &textureManager;
	se::graphics::FontManager &fontManager;
	se::graphics::ModelDataManager &modelDataManager;
	se::graphics::ShapeGenerator &shapeGenerator;
	se::debug::ImmediateModeGraphics &imGraphics;

	MutationDatabase& mutationDatabase;
	UserSettings& userSettings;

	// GUI
	se::gui::GUIView& guiView;

	// ImGui
	se::imgui::BackendWrapper &imguiBackend;

	// Audio
	se::audio::AudioEngine &audioEngine;
	se::audio::AudioManager &audioManager;


	MaterialManager& materialManager;
	SoundPlayer& soundPlayer;
};
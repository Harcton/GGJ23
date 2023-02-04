#pragma once

#include "SpehsEngine/Core/ScopedConnections.h"
#include "SpehsEngine/Graphics/Camera.h"
#include "SpehsEngine/Input/Event.h"


struct DemoContext;

class CameraController
{
public:
	CameraController(DemoContext& _context, int _inputPriority);

	void update();
	glm::vec3 getFrustumPoint(const glm::vec3& _screenCoordinates) const;

private:
	void preUpdateCallback();
	void postUpdateCallback();
	bool mouseHoverCallback(const se::input::MouseHoverEvent& _event);
	bool keyboardCallback(const se::input::KeyboardEvent& _event);
	bool mouseButtonCallback(const se::input::MouseButtonEvent& _event);
	bool mouseMotionCallback(const se::input::MouseMotionEvent& _event);
	bool handleMovementInputs() const;

	DemoContext& context;
	const se::graphics::Camera initialState;

	glm::vec3 movement = glm::vec3(0.0f);
	glm::vec2 rotation = glm::vec2(0.0f);
	float tilt = 0.0f;
	bool mouseMovementActive = false;
	bool receivingHover = false;
	bool boosting = false;
	bool lockYAxis = true;
	glm::ivec2 savedMousePos;

	se::ScopedConnections connections;
};

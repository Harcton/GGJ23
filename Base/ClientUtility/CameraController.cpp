#include "stdafx.h"
#include "Base/ClientUtility/CameraController.h"

#include "boost/bind.hpp"
#include "SpehsEngine/Input/MouseUtilityFunctions.h"
#include "SpehsEngine/Input/EventSignaler.h"
#include "SpehsEngine/Core/DeltaTimeSystem.h"
#include "SpehsEngine/Graphics/Window.h"
#include "Base/DemoContext.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/common.hpp"
#include "glm/gtx/rotate_vector.hpp"


CameraController::CameraController(DemoContext& _context, int _inputPriority)
	: context(_context)
	, initialState(_context.camera)
{
	context.eventSignaler.connectToPreUpdateSignal(
		connections.add(), [this](){ return preUpdateCallback(); });
	context.eventSignaler.connectToPostUpdateSignal(
		connections.add(), [this](){ return postUpdateCallback(); });
	context.eventSignaler.connectToKeyboardSignal(
		connections.add(), [this](const auto& _param){ return keyboardCallback(_param); }, _inputPriority);
	context.eventSignaler.connectToMouseButtonSignal(
		connections.add(), [this](const auto& _param){ return mouseButtonCallback(_param); }, _inputPriority);
	context.eventSignaler.connectToMouseMotionSignal(
		connections.add(), [this](const auto& _param){ return mouseMotionCallback(_param); }, _inputPriority);
	context.eventSignaler.connectToMouseHoverSignal(
		connections.add(), [this](const auto& _param){ return mouseHoverCallback(_param); }, _inputPriority);
}

void CameraController::update()
{
	static constexpr float MOVEMENT_SPEED = 12.0f;
	static constexpr float ROTATION_SPEED = 0.08f;
	static constexpr float TILT_SPEED = 0.4f;

	float moveSpeed = MOVEMENT_SPEED;
	if (boosting)
	{
		moveSpeed *= 5.0f;
	}

	const glm::vec3 position =
		context.camera.getPosition() + movement * moveSpeed * context.deltaTimeSystem.deltaSeconds;
	const glm::vec3 direction =
		glm::rotate(
			glm::rotate(
				context.camera.getDirection(),
				rotation.y * ROTATION_SPEED * context.deltaTimeSystem.deltaSeconds,
				context.camera.getLeft()),
			rotation.x * ROTATION_SPEED * context.deltaTimeSystem.deltaSeconds,
			-context.camera.getUp());

	context.camera.setPosition(position);
	context.camera.setDirection(direction);
	if (!lockYAxis)
	{
		const glm::vec3 unlockedUp =
			glm::rotate(
				glm::normalize(glm::cross(direction, context.camera.getLeft())),
				tilt * TILT_SPEED * context.deltaTimeSystem.deltaSeconds,
				direction);
		context.camera.setUp(unlockedUp);
	}

	if (mouseMovementActive)
	{
		se::input::setMousePosition({ context.mainWindow.getWidth() / 2, context.mainWindow.getHeight() / 2 });
	}
}
glm::vec3 CameraController::getFrustumPoint(const glm::vec3& _screenCoordinates) const
{
#pragma warning(push)
#pragma warning(disable : 4127)
	const glm::vec3 screenCoordinatesFlipped(
		_screenCoordinates.x,
		float(context.mainWindow.getHeight()) - _screenCoordinates.y,
		_screenCoordinates.z);
	return glm::unProject(
		screenCoordinatesFlipped,
		context.camera.getViewMatrix(),
		context.camera.getProjectionMatrix(
			context.mainWindow.getWidth(),
			context.mainWindow.getHeight()),
		glm::ivec4(0, 0, context.mainWindow.getWidth(), context.mainWindow.getHeight()));
#pragma warning(pop)
}

void CameraController::preUpdateCallback()
{
	receivingHover = false;
	movement = glm::mix(movement, glm::vec3(0.0f), 0.2f);
	rotation = glm::mix(rotation, glm::vec2(0.0f), 0.2f);
	tilt = glm::mix(tilt, 0.0f, 0.2f);
	boosting = false;
}
void CameraController::postUpdateCallback()
{
	if (glm::length(movement) > 1.0f)
		movement = glm::normalize(movement);
}
bool CameraController::keyboardCallback(const se::input::KeyboardEvent& _event)
{
	if (!handleMovementInputs())
		return false;

	if (_event.type == se::input::KeyboardEvent::Type::Press)
	{
		if (_event.key == se::input::Key::BACKSPACE)
		{
			context.camera = initialState;
			return true;
		}
	}
	else if (_event.type == se::input::KeyboardEvent::Type::Hold)
	{
		switch (_event.key)
		{
			case se::input::Key::W:
				movement += context.camera.getDirection();
				return true;
			case se::input::Key::S:
				movement += -context.camera.getDirection();
				return true;
			case se::input::Key::A:
				movement += context.camera.getLeft();
				return true;
			case se::input::Key::D:
				movement += -context.camera.getLeft();
				return true;
			case se::input::Key::R:
				movement += context.camera.getUp();
				return true;
			case se::input::Key::F:
				movement += -context.camera.getUp();
				return true;
			case se::input::Key::Q:
				tilt -= 1.0f;
				return true;
			case se::input::Key::E:
				tilt += 1.0f;
				return true;
			case se::input::Key::LSHIFT:
				boosting = true;
				return true;
		}
	}
	return false;
}
bool CameraController::mouseButtonCallback(const se::input::MouseButtonEvent& _event)
{
	if (_event.type == se::input::MouseButtonEvent::Type::Press)
	{
		if (!context.mainWindow.getMouseFocus())
			return false;
		switch (_event.button)
		{
			case se::input::MouseButton::right:
				savedMousePos = se::input::getMousePosition().value();
				mouseMovementActive = true;
				se::input::setShowCursor(false);
				return true;
		}
	}
	else if (_event.type == se::input::MouseButtonEvent::Type::Release)
	{
		switch (_event.button)
		{
			case se::input::MouseButton::right:
				se::input::setMousePosition(savedMousePos);
				mouseMovementActive = false;
				se::input::setShowCursor(true);
				return true;
		}
	}
	return false;
}
bool CameraController::mouseMotionCallback(const se::input::MouseMotionEvent& _event)
{
	if (!handleMovementInputs())
		return false;

	rotation += _event.position - glm::vec2(context.mainWindow.getWidth() / 2.0f, context.mainWindow.getHeight() / 2.0f);
	return true;
}
bool CameraController::mouseHoverCallback(const se::input::MouseHoverEvent&)
{
	if (!context.mainWindow.getMouseFocus())
		return false;
	if (mouseMovementActive)
	{
		receivingHover = true;
		return true;
	}
	return false;
}
bool CameraController::handleMovementInputs() const
{
	if (!context.mainWindow.getMouseFocus() || !mouseMovementActive || !receivingHover)
		return false;
	return true;
}
#include "stdafx.h"
#include "Client/PlayerCharacter.h"

#include "SpehsEngine/Graphics/Animator.h"
#include "SpehsEngine/Graphics/Model.h"
#include "SpehsEngine/Graphics/ModelDataManager.h"
#include "SpehsEngine/Graphics/ShaderManager.h"
#include "SpehsEngine/Graphics/Shape.h"
#include "SpehsEngine/Graphics/TextureManager.h"
#include "SpehsEngine/Input/EventSignaler.h"
#include "Base/Net/Packets.h"
#include "Base/ClientUtility/MaterialManager.h"
#include "Base/ClientUtility/CameraController.h"
#include "Client/BulletManager.h"

using namespace se::graphics;


struct PlayerCharacter::Impl
{
	Impl(ClientContext& _context, BulletManager& _bulletManager);
	~Impl() = default;
	void update();
	void initPlayer(Shape& _shape, bool _remote);

	ClientContext& context;
	BulletManager& bulletManager;
	se::ScopedConnections connections;

	Shape model;
	glm::vec3 input{};
	glm::vec3 movement{};
	glm::vec3 facing{0.0f, 0.0f, -1.0f};
	se::time::Time lastSendUpdateTime;

	std::unordered_map<ClientId, std::unique_ptr<Shape>> remoteClients;
};

PlayerCharacter::PlayerCharacter(ClientContext& _context, BulletManager& _bulletManager)
	: impl(std::make_unique<Impl>(_context, _bulletManager))
{}
PlayerCharacter::~PlayerCharacter()
{}
void PlayerCharacter::update()
{
	impl->update();
}


constexpr glm::vec3 cameraDistance{ 4.0f, 50.0f, 4.0f };

PlayerCharacter::Impl::Impl(ClientContext& _context, BulletManager& _bulletManager)
	: context(_context)
	, bulletManager(_bulletManager)
{
	glm::vec2 playerPos = se::rng::circle(40.0f);

	initPlayer(model, false);
	model.setPosition(toVec3(playerPos));

	context.camera.setPosition(model.getPosition() + cameraDistance);
	context.camera.setDirection(glm::normalize(model.getPosition() - context.camera.getPosition()));

	context.eventSignaler.connectToKeyboardSignal(
		connections.add(),
		[this](const se::input::KeyboardEvent& _event)
		{
			if (_event.type == se::input::KeyboardEvent::Type::Press &&
				_event.key == se::input::Key::SPACE)
			{
				bulletManager.shoot(model.getPosition(), facing);
				return true;
			}

			if (_event.type != se::input::KeyboardEvent::Type::Hold)
				return false;
			switch (_event.key)
			{
				case se::input::Key::W: input += context.camera.getDirection(); return true;
				case se::input::Key::S: input -= context.camera.getDirection(); return true;
				case se::input::Key::A: input += context.camera.getLeft();		return true;
				case se::input::Key::D: input -= context.camera.getLeft();		return true;
			}
			return false;
		}, 1000);
	context.eventSignaler.connectToMouseButtonSignal(
		connections.add(),
		[this](const se::input::MouseButtonEvent& _event)
		{
			if (_event.button == se::input::MouseButton::left &&
				_event.type == se::input::MouseButtonEvent::Type::Press)
			{
				bulletManager.shoot(model.getPosition(), facing);
				return true;
			}
			return false;
		}, 1000);

	context.packetman.registerReceiveHandler<PlayerUpdatesPacket>(PacketType::PlayerUpdates, connections.add(),
		[this](PlayerUpdatesPacket& _packet, const bool _reliable)
		{
			for (const auto& [id, packet] : _packet.playerUpdatePackets)
			{
				if (id == context.myClientId)
					continue; // no need to sync self... ?

				auto it = remoteClients.find(id);
				if (it == remoteClients.end())
				{
					//se::log::info("New remote client created.");
					std::unique_ptr<Shape>& shape = remoteClients[id] = std::make_unique<Shape>();
					initPlayer(*shape, true);
					shape->setPosition({ packet.position.x, 0.0f, packet.position.y });
				}
				else
				{
					const glm::vec3 newPos = toVec3(packet.position);
					//se::log::info("remote pos: " + se::toString(id) + " " + se::toString(newPos));
					const glm::vec3 diff{ newPos - it->second->getPosition() };
					if (glm::length(diff) > 0.0f)
					{
						it->second->setRotation(glm::slerp(
							it->second->getRotation(),
							glm::quatLookAt(glm::normalize(diff), glm::vec3{ 0.0f, 1.0f, 0.0f }),
							0.3f));
					}
					it->second->setPosition(glm::mix(
						it->second->getPosition(),
						newPos,
						0.2f));
				}
			}
		});
}
void PlayerCharacter::Impl::update()
{
	if (glm::length(input) > 0.0f)
	{
		input.y = 0.0f;
		input = glm::normalize(input);
		constexpr float acceleration = 50.0f;
		constexpr float maxSpeed = 30.0f;
		movement += input * acceleration * context.deltaTimeSystem.deltaSeconds;
		input = {};

		const float len = glm::length(movement);
		facing = glm::normalize(movement);
		movement = facing * glm::min(len, maxSpeed);
		model.setRotation(glm::quatLookAt(facing, glm::vec3{ 0.0f, 1.0f, 0.0f }));
	}

	glm::vec3 position = model.getPosition() + movement * context.deltaTimeSystem.deltaSeconds;
	position.y = 2.0f + fabsf(sinf(se::time::now().asSeconds() * 0.001f));
	model.setPosition(position);

	constexpr float deceleration = 1.0f;
	movement = glm::mix(movement, glm::vec3{}, deceleration * context.deltaTimeSystem.deltaSeconds);

	context.camera.setPosition(glm::mix(context.camera.getPosition(), model.getPosition() + facing * 10.0f + cameraDistance, 4.0f * context.deltaTimeSystem.deltaSeconds));
	//context.camera.setDirection(glm::normalize(model.getPosition() - context.camera.getPosition()));

	if (se::time::timeSince(lastSendUpdateTime) > se::time::fromSeconds(1.0f / 20.0f))
	{
		PlayerUpdatePacket packet;
		packet.position.x = position.x;
		packet.position.y = position.z;
		context.packetman.sendPacket(PacketType::PlayerUpdate, packet, false);
		lastSendUpdateTime = se::time::now();
	}
}
void PlayerCharacter::Impl::initPlayer(Shape& _shape, bool _remote)
{
	_shape.generate(ShapeType::Box, ShapeParameters{}, &context.shapeGenerator);
	_shape.setMaterial(context.materialManager.createMaterial(DefaultMaterialType::FlatColor));
	_shape.setColor(_remote ? se::Color(se::CadetBlue) : se::Color(se::BlueViolet));
	_shape.setScale(glm::vec3{ 1.5f, 4.0f, 1.5f });
	context.scene.add(_shape);
}


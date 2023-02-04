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
#include "Base/PlayerAttributes.h"
#include "Base/MutationDatabase.h"
#include "Client/BulletManager.h"
#include <glm/gtx/rotate_vector.hpp>
#pragma optimize("", off)
using namespace se::graphics;


struct PlayerCharacter::Impl
{
	struct PlayerModel
	{
		Model modelBottom;
		Model modelTop;
	};


	Impl(ClientContext& _context, BulletManager& _bulletManager);
	~Impl() = default;
	void update();
	void initPlayer(PlayerModel& _model, bool _remote);
	void updatePlayerAttributes();
	void shoot();

	ClientContext& context;
	BulletManager& bulletManager;
	se::ScopedConnections connections;

	PlayerModel model;
	glm::vec3 input{};
	glm::vec3 movement{};
	glm::vec3 facing{0.0f, 0.0f, -1.0f};
	se::time::Time lastSendUpdateTime;
	PlayerAttributes playerAttributes;
	se::time::Time lastShootTime;

	std::unordered_map<ClientId, std::unique_ptr<PlayerModel>> remoteClients;
	std::vector<std::pair<MutationId, uint16_t>> mutations;
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
	model.modelBottom.setPosition(toVec3(playerPos));
	model.modelTop.setPosition(toVec3(playerPos));

	context.camera.setPosition(model.modelTop.getPosition() + cameraDistance);
	context.camera.setDirection(glm::normalize(model.modelTop.getPosition() - context.camera.getPosition()));

	context.eventSignaler.connectToKeyboardSignal(
		connections.add(),
		[this](const se::input::KeyboardEvent& _event)
		{
			if (_event.key == se::input::Key::SPACE && !_event.isRelease())
			{
				shoot();
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
			if (_event.button == se::input::MouseButton::left && !_event.isRelease())
			{
				shoot();
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
					std::unique_ptr<PlayerModel>& remoteModel = remoteClients[id] = std::make_unique<PlayerModel>();
					initPlayer(*remoteModel, true);
					remoteModel->modelBottom.setPosition({ packet.position.x, 0.0f, packet.position.y });
					remoteModel->modelTop.setPosition({ packet.position.x, 0.0f, packet.position.y });
				}
				else
				{
					const glm::vec3 newPos = toVec3(packet.position);
					//se::log::info("remote pos: " + se::toString(id) + " " + se::toString(newPos));
					const glm::vec3 diff{ newPos - it->second->modelBottom.getPosition() };
					if (glm::length(diff) > 0.0f)
					{
						it->second->modelBottom.setRotation(glm::slerp(
							it->second->modelBottom.getRotation(),
							glm::quatLookAt(glm::normalize(diff), glm::vec3{ 0.0f, 1.0f, 0.0f }),
							0.3f));
					}
					it->second->modelBottom.setPosition(glm::mix(
						it->second->modelBottom.getPosition(),
						newPos,
						0.2f));
					it->second->modelTop.setPosition(glm::mix(
						it->second->modelBottom.getPosition(),
						newPos,
						0.2f));
				}
			}
		});

	context.packetman.registerReceiveHandler<PlayerMutatePacket>(PacketType::PlayerMutated, connections.add(),
		[this](PlayerMutatePacket& _packet, const bool _reliable)
		{
			if (const Mutation* const addedMutation = context.mutationDatabase.find(_packet.mutationId))
			{
				if (addedMutation->rootStrain)
				{
					for (size_t i = 0; i < mutations.size(); i++)
					{
						if (const Mutation* const oldMutation = context.mutationDatabase.find(mutations[i].first))
						{
							if (oldMutation->mutationCategory == MutationCategory::Loadout)
							{
								mutations.erase(mutations.begin() + i--);
							}
						}
					}
					model.modelTop.setColor(toColor(*addedMutation->rootStrain));
				}
			}
			for (std::pair<MutationId, uint16_t>& pair : mutations)
			{
				if (pair.first == _packet.mutationId)
				{
					pair.second += _packet.stacks;
					updatePlayerAttributes();
					return;
				}
			}
			mutations.push_back(std::make_pair(_packet.mutationId, _packet.stacks));
			updatePlayerAttributes();
		});
}
void PlayerCharacter::Impl::update()
{
	if (glm::length(input) > 0.0f)
	{
		input.y = 0.0f;
		input = glm::normalize(input);
		const float acceleration = playerAttributes.movementSpeed * (5.0f / 3.0f);
		movement += input * acceleration * context.deltaTimeSystem.deltaSeconds;
		input = {};

		const float len = glm::length(movement);
		facing = glm::normalize(movement);
		movement = facing * glm::min(len, playerAttributes.movementSpeed);
		model.modelBottom.setRotation(glm::quatLookAt(facing, glm::vec3{ 0.0f, 1.0f, 0.0f }));
	}

	glm::vec3 position = model.modelBottom.getPosition() + movement * context.deltaTimeSystem.deltaSeconds;
	model.modelBottom.setPosition(position);
	model.modelTop.setPosition(position);

	constexpr float deceleration = 1.0f;
	movement = glm::mix(movement, glm::vec3{}, deceleration * context.deltaTimeSystem.deltaSeconds);

	context.camera.setPosition(glm::mix(context.camera.getPosition(), model.modelTop.getPosition() + facing * 10.0f + cameraDistance, 4.0f * context.deltaTimeSystem.deltaSeconds));
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
void PlayerCharacter::Impl::initPlayer(PlayerModel& _model, bool _remote)
{
	auto mat = context.materialManager.createMaterial(DefaultMaterialType::Phong);
	mat->setTexture(context.textureManager.find("white_color"), PhongTextureType::Color);
	mat->setTexture(context.textureManager.find("flat_normal"), PhongTextureType::Normal);

	constexpr glm::vec3 playerModelScale{ 300.0f };

	_model.modelBottom.loadModelData(
		context.modelDataManager.create("player_bottom", "Character_MechTank_1_Bottom.fbx"));
	_model.modelBottom.setScale(playerModelScale);
	_model.modelBottom.setMaterial(mat);
	_model.modelBottom.setColor(_remote ? se::Color(se::CadetBlue) : se::Color(se::BlueViolet));
	context.scene.add(_model.modelBottom);

	_model.modelTop.loadModelData(
		context.modelDataManager.create("player_top", "Character_MechTank_1_Top.fbx"));
	_model.modelTop.setScale(playerModelScale);
	_model.modelTop.setMaterial(mat);
	_model.modelTop.setColor(_remote ? se::Color(se::CadetBlue) : se::Color(se::BlueViolet));
	context.scene.add(_model.modelTop);
}

void PlayerCharacter::Impl::updatePlayerAttributes()
{
	playerAttributes = PlayerAttributes();
	for (const std::pair<MutationId, uint16_t>& pair : mutations)
	{
		if (const Mutation* const mutation = context.mutationDatabase.find(pair.first))
		{
			const uint16_t stacks = std::min(mutation->maxStacks, pair.second);
			mutation->function(playerAttributes, stacks);
		}
	}
}

void PlayerCharacter::Impl::shoot()
{
	se::time::Time weaponInterval = se::time::fromSeconds(0.5f) / playerAttributes.weaponRate;
	if (se::time::timeSince(lastShootTime) > weaponInterval)
	{
		if (playerAttributes.weaponShotSize > 1)
		{
			const glm::vec3 normal(0.0f, 1.0f, 0.0f);
			float angle = -0.5f * playerAttributes.weaponSpread;
			const float angleInterval = playerAttributes.weaponSpread / float(playerAttributes.weaponShotSize - 1);
			for (size_t i = 0; i < playerAttributes.weaponShotSize; i++)
			{
				const glm::vec3 direction = glm::rotate(facing, angle, normal);
				bulletManager.shoot(model.modelTop.getPosition(), direction, playerAttributes.weaponRange, playerAttributes.weaponVelocity, playerAttributes.weaponDamage, playerAttributes.rootStrainLoadout);
				angle += angleInterval;
			}
		}
		else
		{
			bulletManager.shoot(model.modelTop.getPosition(), facing, playerAttributes.weaponRange, playerAttributes.weaponVelocity, playerAttributes.weaponDamage, playerAttributes.rootStrainLoadout);
		}
		lastShootTime = se::time::now();
	}
}
#include "stdafx.h"
#include "Base/Client/PlayerCharacter.h"

#include "SpehsEngine/Graphics/Animator.h"
#include "SpehsEngine/Graphics/Model.h"
#include "SpehsEngine/Graphics/ModelDataManager.h"
#include "SpehsEngine/Graphics/ShaderManager.h"
#include "SpehsEngine/Graphics/Window.h"
#include "SpehsEngine/Graphics/Shape.h"
#include "SpehsEngine/Graphics/Lights.h"
#include "SpehsEngine/Graphics/TextureManager.h"
#include "SpehsEngine/Input/EventSignaler.h"
#include "SpehsEngine/Physics/3D/Ray3D.h"
#include "SpehsEngine/Physics/3D/AABBCollider3D.h"
#include "SpehsEngine/Physics/3D/Collision3D.h"
#include "Base/Net/Packets.h"
#include "Base/ClientUtility/MaterialManager.h"
#include "Base/ClientUtility/CameraController.h"
#include "Base/Client/BulletManager.h"
#include "Base/PlayerAttributes.h"
#include "Base/MutationDatabase.h"
#include "Base/ClientUtility/SoundPlayer.h"
#include <glm/gtx/rotate_vector.hpp>
#pragma optimize("", off)
using namespace se::graphics;


struct PlayerCharacter::Impl
{
	class PlayerModel
	{
		Model modelBottom;
		Model modelTop;
		Model modelWeapon;
		SpotLight light;
		glm::vec3 facing{ 0.0f, 0.0f, -1.0f };

	public:
		void setPlayerColor(const se::Color& _color)
		{
			// TODO: should color material 0 only
			modelBottom.setColor(_color);
			modelTop.setColor(_color);
		}
		void setWeaponColor(const se::Color& _color)
		{
			modelWeapon.setColor(_color);
		}
		const glm::vec3& getPosition() const
		{
			return modelBottom.getPosition();
		}
		const glm::vec3& getWeaponPosition() const
		{
			return getPosition();
		}
		void setPosition(const glm::vec3& _pos)
		{
			modelBottom.setPosition(_pos);
			modelTop.setPosition(_pos);
			modelWeapon.setPosition(_pos);
			light.setPosition(_pos + glm::vec3{ 0.0f, 2.5f, 0.0f });
		}
		void setRotation(const glm::quat& _rot)
		{
			modelBottom.setRotation(_rot);
		}
		const glm::quat& getRotation() const
		{
			return modelBottom.getRotation();
		}
		const glm::vec3& getFacing() const
		{
			return facing;
		}
		void setFacing(const glm::vec3& _dir)
		{
			facing = _dir;
			modelTop.setRotation(glm::quatLookAt(-facing, glm::vec3{ 0.0f, 1.0f, 0.0f }));
			modelWeapon.setRotation(glm::quatLookAt(-facing, glm::vec3{ 0.0f, 1.0f, 0.0f }));
			light.setDirection(facing);
		}
		void init(ClientContext& context)
		{
			constexpr glm::vec3 playerModelScale{ 3.0f };

			auto colorfulMat = context.materialManager.getDefaultMaterial();
			colorfulMat->setTexture(context.textureManager.create("color_gradient_dif.png", "color_gradient_dif.png"), PhongTextureType::Color);
			colorfulMat->setTexture(context.textureManager.find("flat_normal"), PhongTextureType::Normal);

			auto playerMat = context.materialManager.getDefaultMaterial();
			playerMat->setTexture(context.textureManager.create("player_color_dif.png", "player_color_dif.png"), PhongTextureType::Color);
			playerMat->setTexture(context.textureManager.find("flat_normal"), PhongTextureType::Normal);

			auto weaponMat = context.materialManager.getDefaultMaterial();
			weaponMat->setTexture(context.textureManager.create("weapon_color_dif.png", "weapon_color_dif.png"), PhongTextureType::Color);
			weaponMat->setTexture(context.textureManager.find("flat_normal"), PhongTextureType::Normal);
			weaponMat->setTexture(context.textureManager.find("roughness_0"), PhongTextureType::Roughness);

			modelBottom.loadModelData(
				context.modelDataManager.create("player_bottom", "mechtank_lower.fbx"));
			modelBottom.setScale(playerModelScale);
			modelBottom.setMaterial(playerMat, 0);
			modelBottom.setMaterial(colorfulMat, 1);
			context.scene.add(modelBottom);

			modelTop.loadModelData(
				context.modelDataManager.create("player_top", "mechtank_upper.fbx"));
			modelTop.setScale(playerModelScale);
			modelTop.setMaterial(playerMat, 0);
			modelTop.setMaterial(colorfulMat, 1);
			context.scene.add(modelTop);

			modelWeapon.loadModelData(
				context.modelDataManager.create("player_weapon", "mechtank_weapon.fbx"));
			modelWeapon.setScale(playerModelScale);
			modelWeapon.setMaterial(weaponMat, 0);
			modelWeapon.setMaterial(colorfulMat, 1);
			context.scene.add(modelWeapon);

			light.setCone(se::PI<float> *0.4f, se::PI<float> *0.6f);
			light.setRadius(30.0f, 150.0f);
			context.scene.add(light);
		}
	};


	Impl(ClientContext& _context, BulletManager& _bulletManager);
	~Impl() = default;
	void update();
	void updatePlayerAttributes();
	void shoot();

	ClientContext& context;
	BulletManager& bulletManager;
	se::ScopedConnections connections;

	PlayerModel model;
	glm::vec3 input{};
	glm::vec3 movement{};
	se::time::Time lastSendUpdateTime;
	PlayerAttributes playerAttributes;
	se::time::Time lastShootTime;
	glm::vec3 mouseWorldPoint{};

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


constexpr glm::vec3 cameraDistance{ 0.0f, 65.0f, 20.0f };

static glm::vec3 getFrustumPoint(ClientContext& context, const glm::vec3& _screenCoordinates)
{
	const glm::vec3 screenCoordinatesFlipped(
		_screenCoordinates.x,
		float(context.mainWindow.getHeight()) - _screenCoordinates.y,
		_screenCoordinates.z);
	return glm::unProject(
		screenCoordinatesFlipped,
		context.camera.getViewMatrix(),
		context.camera.getProjectionMatrix(context.mainWindow.getWidth(), context.mainWindow.getHeight()),
		glm::ivec4(0, 0, context.mainWindow.getWidth(), context.mainWindow.getHeight()));
}

PlayerCharacter::Impl::Impl(ClientContext& _context, BulletManager& _bulletManager)
	: context(_context)
	, bulletManager(_bulletManager)
{
	glm::vec2 playerPos = se::rng::circle(40.0f);

	model.init(context);
	model.setPosition(toVec3(playerPos));

	context.camera.setPosition(model.getPosition() + cameraDistance);
	context.camera.setDirection(glm::normalize(model.getPosition() - context.camera.getPosition()));

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

	context.eventSignaler.connectToMouseHoverSignal(
		connections.add(),
		[this](const se::input::MouseHoverEvent& _event)
		{
			const glm::vec3 screenPoint = getFrustumPoint(context, glm::vec3(_event.position, 0.0f));
			const se::physics::Ray3D mouseRay(screenPoint,
				getFrustumPoint(context, glm::vec3(_event.position, 1.0f)));

			const se::physics::Collision3D raycast(
				mouseRay,
				se::physics::AABBCollider3D(
					glm::vec3{ 0.0f, -1.0f, 0.0f },
					glm::vec3{ 10000.0f, 1.0f, 10000.0f }));
			se_assert(raycast.hit());
			mouseWorldPoint = raycast.point();
			return true;
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
					std::unique_ptr<PlayerModel>& remoteModel = remoteClients[id] = std::make_unique<PlayerModel>();
					remoteModel->init(context);
					remoteModel->setPosition(toVec3(packet.position));
					remoteModel->setFacing(toVec3(packet.facing));
					if (const se::Color* const color = tryFind(context.clientColors, id))
					{
						remoteModel->setPlayerColor(*color);
					}
				}
				else
				{
					const glm::vec3 newPos = toVec3(packet.position);
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
						0.4f));
					it->second->setFacing(glm::mix(
						it->second->getFacing(),
						toVec3(packet.facing),
						0.4f));
					it->second->setWeaponColor(toColor(packet.rootStrainLoadout));
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
				}
				else
				{
					context.soundPlayer.playSound("shooter_side_upgrade.ogg", model.getPosition());
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

	playerAttributes.rootStrainLoadout = context.startingRootStrain;
	model.setWeaponColor(toColor(context.startingRootStrain));
	if (const se::Color* const color = tryFind(context.clientColors, context.myClientId))
	{
		model.setPlayerColor(*color);
	}
}
void PlayerCharacter::Impl::update()
{
	if (glm::length(input) > 0.0f)
	{
		input.y = 0.0f;
		input = glm::normalize(input);
		const float acceleration = playerAttributes.movementSpeed * 3.0f;
		movement += input * acceleration * context.deltaTimeSystem.deltaSeconds;
		input = {};

		const float len = glm::length(movement);
		movement = glm::normalize(movement) * glm::min(len, playerAttributes.movementSpeed);
		model.setRotation(glm::quatLookAt(glm::normalize(movement), glm::vec3{ 0.0f, 1.0f, 0.0f }));
	}

	glm::vec3 position = model.getPosition() + movement * context.deltaTimeSystem.deltaSeconds;
	if (glm::distance(position, glm::vec3{}) > (constants::worldSize * 0.5f))
	{
		position = glm::normalize(position) * (constants::worldSize * 0.5f);
	}
	if (glm::distance(position, glm::vec3{}) < (constants::coreSize * 0.5f))
	{
		position = glm::normalize(position) * (constants::coreSize * 0.5f);
	}
	model.setPosition(position);

	model.setFacing(glm::normalize(mouseWorldPoint - model.getPosition()));

	constexpr float deceleration = 1.0f;
	movement = glm::mix(movement, glm::vec3{}, deceleration * context.deltaTimeSystem.deltaSeconds);

	context.camera.setPosition(glm::mix(
		context.camera.getPosition(),
		model.getPosition() + model.getFacing() * 5.0f + cameraDistance,
		5.0f * context.deltaTimeSystem.deltaSeconds));
	//context.camera.setDirection(glm::normalize(model.getPosition() - context.camera.getPosition()));

	if (se::time::timeSince(lastSendUpdateTime) > se::time::fromSeconds(1.0f / 20.0f))
	{
		PlayerUpdatePacket packet;
		packet.position.x = model.getPosition().x;
		packet.position.y = model.getPosition().z;
		packet.facing.x = model.getFacing().x;
		packet.facing.y = model.getFacing().z;
		packet.rootStrainLoadout = playerAttributes.rootStrainLoadout;
		context.packetman.sendPacket(PacketType::PlayerUpdate, packet, false);
		lastSendUpdateTime = se::time::now();
	}
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
	model.setWeaponColor(toColor(playerAttributes.rootStrainLoadout));
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
				const glm::vec3 direction = glm::rotate(model.getFacing(), angle, normal);
				bulletManager.shoot(model.getPosition(), direction, playerAttributes.weaponRange, playerAttributes.weaponVelocity, playerAttributes.weaponDamage, playerAttributes.rootStrainLoadout);
				angle += angleInterval;
			}
		}
		else
		{
			bulletManager.shoot(model.getPosition(), model.getFacing(), playerAttributes.weaponRange, playerAttributes.weaponVelocity, playerAttributes.weaponDamage, playerAttributes.rootStrainLoadout);
		}
		lastShootTime = se::time::now();
	}
}
#include "stdafx.h"
#include "Base/Client/EvilRootManager.h"

#include "SpehsEngine/Graphics/Animator.h"
#include "SpehsEngine/Graphics/Model.h"
#include "SpehsEngine/Graphics/ModelDataManager.h"
#include "SpehsEngine/Graphics/ShaderManager.h"
#include "SpehsEngine/Graphics/Shape.h"
#include "SpehsEngine/Graphics/FontManager.h"
#include "SpehsEngine/Graphics/TextureManager.h"
#include "Base/ClientUtility/MaterialManager.h"
#include "Base/ClientUtility/SoundPlayer.h"
#include "Base/Net/Packets.h"
#include "Base/Client/BulletManager.h"

using namespace se::graphics;
#pragma optimize("", off)


constexpr se::time::Time growthTime = se::time::fromSeconds(2.5f);
constexpr float headRadius = 5.0f;

struct EvilRootManager::Impl
{
	Impl(ClientContext& _context, BulletManager& _bulletManager, float _worldSize);
	~Impl() = default;
	void update();

	ClientContext& context;
	BulletManager& bulletManager;
	const float worldRadius;

	struct EvilRootVisuals
	{
		EvilRootVisuals(ClientContext& _context, RootCreatePacket& _packet,
						BulletManager& _bulletManager)
			: context(_context)
			, bulletManager(_bulletManager)
			, id(_packet.rootId)
			, startPoint(toVec3(_packet.start))
			, endPoint(toVec3(_packet.end))
			, spawnTime(se::time::now())
			, growthDir(glm::normalize(endPoint - startPoint))
			, rootStrain(_packet.rootStrain)
			, health(_packet.health)
		{
			const std::string rootModelName = "root_" + std::to_string(((int)id % 3) + 1) + ".gltf";
			root.loadModelData(context.modelDataManager.create(rootModelName, rootModelName));
			root.setPosition(glm::vec3{ startPoint });
			root.setRotation(glm::quatLookAt(growthDir, glm::vec3{0.0f, 1.0f, 0.0f}));
			root.setColor(constants::rootColor);
			root.setMaterial(context.materialManager.getDefaultMaterial());
			context.scene.add(root);
		}
		void findAndAdd(RootCreatePacket& _packet)
		{
			if (id == _packet.parentRootId)
			{
				context.soundPlayer.playSound("root_grow.ogg", toVec3(_packet.start));
				branches.push_back(std::make_unique<EvilRootVisuals>(context, _packet, bulletManager));
			}
			else
			{
				for (auto&& branch : branches)
				{
					branch->findAndAdd(_packet);
				}
			}
		}
		void onErase()
		{
			context.soundPlayer.playSound("cracking.ogg", endPoint);
			for (auto&& branch : branches)
			{
				branch->onErase();
			}
		}
		void findAndDelete(RootRemovePacket& _packet)
		{
			se_assert(_packet.rootId != id);
			for (size_t i = 0; i < branches.size(); i++)
			{
				if (branches[i]->id == _packet.rootId)
				{
					branches[i]->onErase();
					branches.erase(branches.begin() + i);
					return;
				}
			}
			for (auto&& branch : branches)
			{
				branch->findAndDelete(_packet);
			}
		}
		void update()
		{
			const float rootLength = glm::distance(startPoint, endPoint);
			const float rootScale = glm::distance(startPoint, endPoint) / constants::defaultRootLength;
			const float growthProgress = glm::clamp(se::time::timeSince(spawnTime).asSeconds() / growthTime.asSeconds(), 0.0f, 1.0f);
			root.setScale(glm::vec3{ 1.0f, 1.0f, growthProgress * rootScale });
			root.setPosition(glm::vec3{ startPoint } + growthDir * growthProgress * rootLength * 0.5f);

			if (!head.has_value() && se::time::timeSince(spawnTime) > growthTime)
			{
				head.emplace();
				const std::string jointModelName = "joint_" + std::to_string(((int)id % 2) + 1) + ".gltf";
				head->loadModelData(context.modelDataManager.create(jointModelName, jointModelName));
				head->setPosition(glm::vec3{ endPoint });
				head->setRotation(glm::quatLookAt(growthDir, glm::vec3{ 0.0f, 1.0f, 0.0f }));
				head->setScale(glm::vec3{ 1.2f });
				head->setColor(toColor(rootStrain));
				head->setMaterial(context.materialManager.getDefaultMaterial());
				context.scene.add(*head);

				auto mat = context.materialManager.createMaterial(DefaultMaterialType::Text);
				mat->setFont(context.fontManager.getDefaultFont());
				hpText.setMaterial(mat);
				hpText.setScale(glm::vec3{ 0.05f });
				hpText.insert(std::to_string((int)health));
				hpText.setPosition(endPoint + glm::vec3{ 0.0f, 5.0f, 0.0f });
				context.scene.add(hpText);
			}

			if (head.has_value())
			{
				if (const std::optional<float> damage = bulletManager.hitTest(endPoint, headRadius, rootStrain))
				{
					sendRootDamage(id, *damage);
				}
			}

			for (auto&& branch : branches)
			{
				branch->update();
			}
		}
		void update(RootUpdatePacket& _packet)
		{
			if (id == _packet.rootId)
			{
				health = _packet.health;
				hpText.clear();
				hpText.insert(std::to_string((int)health));
				return;
			}
			for (auto&& branch : branches)
			{
				branch->update(_packet);
			}
		}

		void sendRootDamage(const RootId _rootId, const float _damage)
		{
			RootDamagePacket packet;
			packet.rootId = _rootId;
			packet.damage = _damage;
			context.packetman.sendPacket<RootDamagePacket>(PacketType::RootDamage, packet, true);
		}

		ClientContext& context;
		BulletManager& bulletManager;
		const RootId id;
		const glm::vec3 startPoint;
		const glm::vec3 endPoint;
		const se::time::Time spawnTime;
		const glm::vec3 growthDir;
		const RootStrain rootStrain;
		Model root;
		std::optional<Model> head;
		Text hpText;
		float health = 0.0f;

		std::vector<std::unique_ptr<EvilRootVisuals>> branches;
	};

	std::vector<std::unique_ptr<EvilRootVisuals>> rootData;
	se::time::Time lastSpawned = se::time::Time::zero;
	se::ScopedConnections scopedConnections;
};

EvilRootManager::EvilRootManager(ClientContext& _context, BulletManager& _bulletManager, float _worldSize)
	: impl(std::make_unique<Impl>(_context, _bulletManager, _worldSize)) { }
EvilRootManager::~EvilRootManager(){ }
void EvilRootManager::update()
{
	impl->update();
}


EvilRootManager::Impl::Impl(ClientContext& _context, BulletManager& _bulletManager, float _worldSize)
	: context(_context)
	, bulletManager(_bulletManager)
	, worldRadius(_worldSize * 0.5f)
{
	context.packetman.registerReceiveHandler<RootCreatePacket>(
		PacketType::RootCreate, scopedConnections.add(),
		[this](RootCreatePacket& _packet, const bool _reliable)
		{
			if (_packet.parentRootId == RootId{})
			{
				rootData.push_back(std::make_unique<EvilRootVisuals>(context, _packet, bulletManager));
				context.soundPlayer.playSound("root_grow.ogg", toVec3(_packet.start));
			}
			else
			{
				for (auto&& root : rootData)
				{
					root->findAndAdd(_packet);
				}
			}
		});

	context.packetman.registerReceiveHandler<RootUpdatePacket>(
		PacketType::RootUpdate, scopedConnections.add(),
		[this](RootUpdatePacket& _packet, const bool _reliable)
		{
			for (auto&& root : rootData)
			{
				root->update(_packet);
			}
		});

	context.packetman.registerReceiveHandler<RootRemovePacket>(
		PacketType::RootRemove, scopedConnections.add(),
		[this](RootRemovePacket& _packet, const bool _reliable)
		{
			for (size_t i = 0; i < rootData.size(); i++)
			{
				if (rootData[i]->id == _packet.rootId)
				{
					rootData[i]->onErase();
					rootData.erase(rootData.begin() + i);
					return;
				}
			}
			for (auto&& root : rootData)
			{
				root->findAndDelete(_packet);
			}
		});
}
void EvilRootManager::Impl::update()
{
	for (auto&& root : rootData)
	{
		root->update();
	}
}

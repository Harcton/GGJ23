#include "stdafx.h"
#include "Client/EvilRootManager.h"

#include "SpehsEngine/Graphics/Animator.h"
#include "SpehsEngine/Graphics/Model.h"
#include "SpehsEngine/Graphics/ModelDataManager.h"
#include "SpehsEngine/Graphics/ShaderManager.h"
#include "SpehsEngine/Graphics/Shape.h"
#include "SpehsEngine/Graphics/FontManager.h"
#include "SpehsEngine/Graphics/TextureManager.h"
#include "Base/ClientUtility/MaterialManager.h"
#include "Base/Net/Packets.h"
#include "Client/BulletManager.h"

using namespace se::graphics;
#pragma optimize("", off)


constexpr se::time::Time growthTime = se::time::fromSeconds(4.0f);
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
		EvilRootVisuals(ClientContext& _context, RootCreatePacket& _packet)
			: context(_context)
			, id(_packet.rootId)
			, startPoint(toVec3(_packet.start))
			, endPoint(toVec3(_packet.end))
			, spawnTime(se::time::now())
			, growthDir(glm::normalize(endPoint - startPoint))
			, health(_packet.health)
		{
			root.generate(ShapeType::Box);
			root.setPosition(glm::vec3{ startPoint });
			root.setRotation(glm::quatLookAt(growthDir, glm::vec3{0.0f, 1.0f, 0.0f}));
			root.setColor(se::Color(se::SaddleBrown));
			root.setMaterial(context.materialManager.createMaterial(DefaultMaterialType::FlatColor));
			context.scene.add(root);
		}
		void update()
		{
			const float rootLength = glm::distance(startPoint, endPoint);
			const float growthProgress = glm::clamp(se::time::timeSince(spawnTime).asSeconds() / growthTime.asSeconds(), 0.0f, 1.0f);
			root.setScale(glm::vec3{ 1.0f, 1.0f, growthProgress * rootLength });
			root.setPosition(glm::vec3{ startPoint } + growthDir * growthProgress * rootLength * 0.5f);

			if (!head.has_value() && se::time::timeSince(spawnTime) > growthTime)
			{
				head.emplace();
				head->generate(ShapeType::Sphere);
				head->setPosition(glm::vec3{ endPoint });
				head->setScale(glm::vec3{ headRadius });
				head->setColor(se::Color(se::Red));
				head->setMaterial(context.materialManager.createMaterial(DefaultMaterialType::FlatColor));
				context.scene.add(*head);


				auto mat = context.materialManager.createMaterial(DefaultMaterialType::Text);
				mat->setFont(context.fontManager.getDefaultFont());
				hpText.setMaterial(mat);
				hpText.setScale(glm::vec3{ 0.05f });
				hpText.insert(std::to_string((int)health));
				hpText.setPosition(endPoint + glm::vec3{ 0.0f, 5.0f, 0.0f });
				context.scene.add(hpText);
			}
		}
		void update(RootUpdatePacket& packet)
		{
			se_assert(id == packet.rootId);
			health = packet.health;
			hpText.clear();
			hpText.insert(std::to_string((int)health));
		}

		ClientContext& context;
		const RootId id;
		const glm::vec3 startPoint;
		const glm::vec3 endPoint;
		const se::time::Time spawnTime;
		const glm::vec3 growthDir;
		Shape root;
		std::optional<Shape> head;
		Text hpText;
		float health = 0.0f;
	};

	void sendRootDamage(const RootId _rootId, const float _damage)
	{
		RootDamagePacket packet;
		packet.rootId = _rootId;
		packet.damage = _damage;
		context.packetman.sendPacket<RootDamagePacket>(PacketType::RootDamage, packet, true);
	}

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
			rootData.push_back(std::make_unique<EvilRootVisuals>(context, _packet));
		});

	context.packetman.registerReceiveHandler<RootUpdatePacket>(
		PacketType::RootUpdate, scopedConnections.add(),
		[this](RootUpdatePacket& _packet, const bool _reliable)
		{
			for (auto&& root : rootData)
			{
				if (root->id == _packet.rootId)
				{
					root->update(_packet);
					break;
				}
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
					rootData.erase(rootData.begin() + i);
					break;
				}
			}
		});
}
void EvilRootManager::Impl::update()
{
	for (auto&& root : rootData)
	{
		root->update();
		if (root->head.has_value() && bulletManager.hitTest(root->endPoint, headRadius))
		{
			constexpr float defaultDamage = 10.0f;
			sendRootDamage(root->id, defaultDamage);
		}
	}
}

#include "stdafx.h"
#include "Client/EvilRootManager.h"

#include "SpehsEngine/Graphics/Animator.h"
#include "SpehsEngine/Graphics/Model.h"
#include "SpehsEngine/Graphics/ModelDataManager.h"
#include "SpehsEngine/Graphics/ShaderManager.h"
#include "SpehsEngine/Graphics/Shape.h"
#include "SpehsEngine/Graphics/TextureManager.h"
#include "Base/ClientUtility/MaterialManager.h"
#include "Base/Net/Packets.h"
#include "Client/BulletManager.h"

using namespace se::graphics;


constexpr float rootLength = 20.0f;
constexpr se::time::Time growthTime = se::time::fromSeconds(4.0f);
constexpr se::time::Time growthInterval = se::time::fromSeconds(15.0f);
constexpr float headRadius = 5.0f;

struct EvilRootManager::Impl
{
	Impl(ClientContext& _context, BulletManager& _bulletManager, float _worldSize);
	~Impl() = default;
	void update();

	ClientContext& context;
	BulletManager& bulletManager;
	const float worldRadius;

	struct EvilRootData
	{
		virtual ~EvilRootData() = default;
		EvilRootData(ClientContext& _context, glm::vec3 _start, glm::vec3 _end)
			: context(_context)
			, startPoint(_start)
			, endPoint(_end)
			, spawnTime(se::time::now())
		{}
		virtual void update(BulletManager& _bulletManager)
		{
			if (branches.empty() && se::time::timeSince(spawnTime) > growthInterval)
			{
				const glm::vec3 newEnd = endPoint + glm::normalize(-endPoint) * rootLength;
				createBranch(endPoint, newEnd);
			}

			for (auto it = branches.begin(); it != branches.end();)
			{
				EvilRootData& branch = *it->get();
				if (se::time::timeSince(branch.spawnTime) > growthTime &&
					_bulletManager.hitTest(branch.endPoint, headRadius))
				{
					it = branches.erase(it);
					continue;
				}
				branch.update(_bulletManager);
				it++;
			}
		}
		virtual void createBranch(glm::vec3 _start, glm::vec3 _end) = 0;

		ClientContext& context;
		const glm::vec3 startPoint;
		const glm::vec3 endPoint;
		const se::time::Time spawnTime;
		std::vector<std::unique_ptr<EvilRootData>> branches;
	};
	struct EvilRootVisuals : EvilRootData
	{
		EvilRootVisuals(ClientContext& _context, glm::vec3 _start, glm::vec3 _end)
			: EvilRootData(_context, _start, _end)
			, growthDir(glm::normalize(glm::vec3{ _end - _start }))
		{
			root.generate(ShapeType::Box);
			root.setPosition(glm::vec3{ startPoint });
			root.setRotation(glm::quatLookAt(growthDir, glm::vec3{0.0f, 1.0f, 0.0f}));
			root.setColor(se::Color(se::Pink));
			root.setMaterial(context.materialManager.createMaterial(DefaultMaterialType::FlatColor));
			context.scene.add(root);
		}
		void update(BulletManager& _bulletManager) override
		{
			EvilRootData::update(_bulletManager);

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
			}
		}
		void createBranch(glm::vec3 _start, glm::vec3 _end) override
		{
			branches.push_back(std::make_unique<EvilRootVisuals>(context, _start, _end));
		}

		const glm::vec3 growthDir;
		Shape root;
		std::optional<Shape> joint;
		std::optional<Shape> head;
	};

	void sendRootDamage(const RootId _rootId, const float _damage)
	{
		RootDamagePacket packet;
		packet.rootId = _rootId;
		packet.damage = _damage;
		context.packetman.sendPacket<RootDamagePacket>(PacketType::RootDamage, packet, true);
	}

	std::vector<std::unique_ptr<EvilRootData>> rootData;
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
	context.packetman.registerReceiveHandler<RootCreatePacket>(PacketType::RootCreate, scopedConnections.add(),
		[this](RootCreatePacket& _packet, const bool _reliable)
		{
			se::log::info("Root create TODO");
		});
	context.packetman.registerReceiveHandler<RootUpdatePacket>(PacketType::RootUpdate, scopedConnections.add(),
		[this](RootUpdatePacket& _packet, const bool _reliable)
		{
			se::log::info("Root update TODO");
		});
	context.packetman.registerReceiveHandler<RootRemovePacket>(PacketType::RootRemove, scopedConnections.add(),
		[this](RootRemovePacket& _packet, const bool _reliable)
		{
			se::log::info("Root remove TODO");
		});
}
void EvilRootManager::Impl::update()
{
	constexpr se::time::Time spawnInterval = se::time::fromSeconds(5.0f);
	if (se::time::timeSince(lastSpawned) > spawnInterval)
	{
		const glm::vec2 start2d = se::rng::circle(worldRadius);
		const glm::vec3 start{ start2d.x, 0.0f, start2d.y };
		const glm::vec3 end = start + glm::normalize(-start) * rootLength;
		rootData.push_back(std::make_unique<EvilRootVisuals>(context, start, end));
		lastSpawned = se::time::now();
	}

	for (auto it = rootData.begin(); it != rootData.end();)
	{
		EvilRootData& root = *it->get();
		if (se::time::timeSince(root.spawnTime) > growthTime &&
			bulletManager.hitTest(root.endPoint, headRadius))
		{
			it = rootData.erase(it);
			continue;
		}
		root.update(bulletManager);
		it++;
	}
}

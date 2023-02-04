#include "stdafx.h"
#include "Client/BulletManager.h"

#include "SpehsEngine/Graphics/Animator.h"
#include "SpehsEngine/Graphics/Model.h"
#include "SpehsEngine/Graphics/ModelDataManager.h"
#include "SpehsEngine/Graphics/ShaderManager.h"
#include "SpehsEngine/Graphics/Shape.h"
#include "SpehsEngine/Graphics/TextureManager.h"
#include "SpehsEngine/Input/EventSignaler.h"
#include "Base/ClientUtility/MaterialManager.h"
#include "Base/Net/Packets.h"

using namespace se::graphics;


struct BulletManager::Impl
{
	~Impl() = default;
	Impl(ClientContext& _context, float _worldSize);
	void update();
	void shoot(const glm::vec3& _pos, const glm::vec3& _dir);
	bool hitTest(const glm::vec3& _pos, float _radius);

	ClientContext& context;
	const float worldRadius;

	struct Bullet
	{
		Bullet(ClientContext& _context, const glm::vec3& _pos, const glm::vec3& _dir, const bool _owned)
			: start(_pos), dir(_dir), owned(_owned)
		{
			model.generate(ShapeType::Ball, ShapeParameters{}, &_context.shapeGenerator);
			model.setPosition(start);
			model.setScale(glm::vec3{ 0.5f });
			model.setMaterial(_context.materialManager.createMaterial(DefaultMaterialType::FlatColor));
			model.setColor(se::Color(se::Orange));
			_context.scene.add(model);
		}
		const glm::vec3 start;
		const glm::vec3 dir;
		const bool owned;
		Shape model;
	};
	std::vector<std::unique_ptr<Bullet>> bullets;
	se::ScopedConnections scopedConnections;
};


BulletManager::BulletManager(ClientContext& _context, float _worldSize)
	: impl(std::make_unique<Impl>(_context, _worldSize))
{}
BulletManager::~BulletManager()
{}
void BulletManager::update()
{
	impl->update();
}
void BulletManager::shoot(const glm::vec3& _pos, const glm::vec3& _dir)
{
	impl->shoot(_pos, _dir);
}
bool BulletManager::hitTest(const glm::vec3& _pos, float _radius)
{
	return impl->hitTest(_pos, _radius);
}


BulletManager::Impl::Impl(ClientContext& _context, float _worldSize)
	: context(_context)
	, worldRadius(_worldSize * 0.5f)
{
	_context.packetman.registerReceiveHandler<BulletCreatePacket>(PacketType::BulletCreate, scopedConnections.add(),
		[this](BulletCreatePacket& _packet, const bool _reliable)
		{
			const float height = 2.42f;
			const glm::vec3 position3D(_packet.position2D.x, height, _packet.position2D.y);
			const glm::vec3 direction3D(_packet.direction2D.x, 0.0f, _packet.direction2D.y);
			bullets.push_back(std::make_unique<Bullet>(context, position3D, direction3D, false));
		});
}
void BulletManager::Impl::update()
{
	for (auto it = bullets.begin(); it != bullets.end(); )
	{
		constexpr float bulletSpeed = 100.0f;
		Bullet& bullet = *it->get();
		bullet.model.setPosition(bullet.model.getPosition() + bullet.dir * bulletSpeed * context.deltaTimeSystem.deltaSeconds);
		if (glm::length(bullet.model.getPosition()) > worldRadius + 20.0f)
		{
			it = bullets.erase(it);
			continue;
		}
		it++;
	}
}
void BulletManager::Impl::shoot(const glm::vec3& _pos, const glm::vec3& _dir)
{
	bullets.push_back(std::make_unique<Bullet>(context, _pos, _dir, true));
	BulletCreatePacket packet;
	packet.position2D.x = _pos.x;
	packet.position2D.y = _pos.z;
	packet.direction2D.x = _dir.x;
	packet.direction2D.y = _dir.z;
	context.packetman.sendPacket(PacketType::BulletCreate, packet, false);
}
bool BulletManager::Impl::hitTest(const glm::vec3& _pos, float _radius)
{
	for (auto it = bullets.begin(); it != bullets.end();)
	{
		Bullet& bullet = *it->get();
		if (glm::distance(_pos, bullet.model.getPosition()) < (_radius + 0.5f))
		{
			it = bullets.erase(it);
			// delete remote bullets on impact, but return true for own bullets only
			return bullet.owned;
		}
		it++;
	}
	return false;
}


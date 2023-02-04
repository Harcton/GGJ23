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
#include "Base/ClientUtility/SoundPlayer.h"
#include "Base/ClientUtility/SoundPlayer.h"
#include "Base/Net/Packets.h"

using namespace se::graphics;


struct BulletManager::Impl
{
	~Impl() = default;
	Impl(ClientContext& _context, float _worldSize);
	void update();
	void shoot(const glm::vec3& _pos, const glm::vec3& _dir, const float _range, const float _speed, const float _damage, const RootStrain _rootStrain);
	bool hitTest(const glm::vec3& _pos, float _radius);

	ClientContext& context;
	const float worldRadius;

	struct Bullet
	{
		Bullet(ClientContext& _context, const glm::vec3& _pos, const glm::vec3& _dir, const float _range, const float _speed, const float _damage,
			const RootStrain _rootStrain, const bool _owned)
			: start(_pos), dir(_dir), owned(_owned), range(_range), speed(_speed), damage(_damage), rootStrain(_rootStrain)
		{
			model.generate(ShapeType::Ball, ShapeParameters{}, &_context.shapeGenerator);
			model.setPosition(start + glm::vec3{ 0.0f, 2.5f, 0.0f });
			model.setScale(glm::vec3{ 0.5f });
			model.setMaterial(_context.materialManager.createMaterial(DefaultMaterialType::FlatColor));
			model.setColor(se::Color(toColor(_rootStrain)));
			_context.scene.add(model);
		}
		const glm::vec3 start;
		const glm::vec3 dir;
		const float range;
		const float speed;
		const float damage;
		const RootStrain rootStrain;
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
void BulletManager::shoot(const glm::vec3& _pos, const glm::vec3& _dir, const float _range, const float _speed, const float _damage, const RootStrain _rootStrain)
{
	impl->shoot(_pos, _dir, _range, _speed, _damage, _rootStrain);
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
			const glm::vec3 position3D = toVec3(_packet.position2D);
			const glm::vec3 direction3D = toVec3(_packet.direction2D);
			bullets.push_back(std::make_unique<Bullet>(context, position3D, direction3D, _packet.range, _packet.speed, _packet.damage, _packet.rootStrain, false));
			context.soundPlayer.playSound("gunfire1.ogg", position3D);
		});
}
void BulletManager::Impl::update()
{
	for (auto it = bullets.begin(); it != bullets.end(); )
	{
		Bullet& bullet = *it->get();
		const glm::vec3 position = bullet.model.getPosition() + bullet.dir * bullet.speed * context.deltaTimeSystem.deltaSeconds;
		const float distanceFromStart = glm::length(bullet.start - position);
		if (glm::length(bullet.model.getPosition()) > worldRadius + 20.0f || distanceFromStart >= bullet.range)
		{
			it = bullets.erase(it);
			continue;
		}
		else
		{
			bullet.model.setPosition(position);
			it++;
		}
	}
}
void BulletManager::Impl::shoot(const glm::vec3& _pos, const glm::vec3& _dir, const float _range, const float _speed, const float _damage, const RootStrain _rootStrain)
{
	bullets.push_back(std::make_unique<Bullet>(context, _pos, _dir, _range, _speed, _damage, _rootStrain, true));
	context.soundPlayer.playSound("gunfire1.ogg", _pos);
	BulletCreatePacket packet;
	packet.position2D.x = _pos.x;
	packet.position2D.y = _pos.z;
	packet.direction2D.x = _dir.x;
	packet.direction2D.y = _dir.z;
	packet.range = _range;
	packet.speed = _speed;
	packet.damage = _damage;
	packet.rootStrain = _rootStrain;
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
			context.soundPlayer.playSound("gunhit.ogg", _pos);
			// delete remote bullets on impact, but return true for own bullets only
			return bullet.owned;
		}
		it++;
	}
	return false;
}


#include "stdafx.h"
#include "Server/RootServer.h"

#include "SpehsEngine/Graphics/Animator.h"
#include "SpehsEngine/Graphics/Model.h"
#include "SpehsEngine/Graphics/ModelDataManager.h"
#include "SpehsEngine/Graphics/ShaderManager.h"
#include "SpehsEngine/Graphics/Shape.h"
#include "SpehsEngine/Graphics/TextureManager.h"
#include "Base/ClientUtility/MaterialManager.h"
#include "Client/BulletManager.h"

using namespace se::graphics;


constexpr float rootLength = 20.0f;
constexpr se::time::Time growthTime = se::time::fromSeconds(4.0f);
constexpr se::time::Time growthInterval = se::time::fromSeconds(15.0f);
constexpr float headRadius = 5.0f;

struct RootServer::Impl
{
	RootServer::Impl(ServerContext& _context, float _worldSize)
		: context(_context)
		, worldRadius(_worldSize * 0.5f)
	{
		for (const std::unique_ptr<Client>& client : context.clients)
		{
			client->packetman.registerReceiveHandler<RootDamagePacket>(PacketType::RootDamage, scopedConnections.add(),
				[this](RootDamagePacket& _packet, const bool _reliable)
				{
					rootDamageRecursive(roots, _packet);
				});
		}
	}

	bool rootDamageRecursive(std::vector<Root>& _roots, const RootDamagePacket& _packet)
	{
		for (size_t i = 0; i < _roots.size(); i++)
		{
			if (_roots[i].rootId == _packet.rootId)
			{
				_roots[i].health -= _packet.damage;
				if (_roots[i].health <= 0.0f)
				{
					std::swap(_roots[i], roots.back());
					_roots.pop_back();
					RootRemovePacket packet;
					packet.rootId = _packet.rootId;
					for (const std::unique_ptr<Client>& client : context.clients)
					{
						client->packetman.sendPacket<RootRemovePacket>(PacketType::RootRemove, packet, true);
					}
				}
				else
				{
					queueUpdateRootIds.insert(_packet.rootId);
				}
				return true;
			}
			else if (rootDamageRecursive(_roots[i].children, _packet))
			{
				return true;
			}
		}
		return false;
	}

	const Root *findRoot(std::vector<Root>& _roots, const RootId _rootId)
	{
		for (size_t i = 0; i < _roots.size(); i++)
		{
			if (_roots[i].rootId == _rootId)
			{
				return &_roots[i];
			}
			else if (const Root* const result = findRoot(_roots[i].children, _rootId))
			{
				return result;
			}
		}
		return nullptr;
	}

	void update()
	{
		constexpr se::time::Time spawnInterval = se::time::fromSeconds(5.0f);
		if (se::time::timeSince(lastSpawnTime) > spawnInterval)
		{
			Root root;
			root.rootId = nextRootId.value++;
			root.start = se::rng::circle(worldRadius);
			root.end = root.start + glm::normalize(-root.start) * rootLength;
			root.health = 100.0f;
			roots.push_back(root);

			sendRootCreate(root, nullptr);

			lastSpawnTime = se::time::now();
		}
		if (se::time::timeSince(lastUpdateTime) > se::time::fromSeconds(1.0f / 5.0f))
		{
			if (!queueUpdateRootIds.empty())
			{
				for (const RootId rootId : queueUpdateRootIds)
				{
					if (const Root* const root = findRoot(roots, rootId))
					{
						RootUpdatePacket packet;
						packet.rootId = rootId;
						packet.health = root->health;
						for (const std::unique_ptr<Client>& client : context.clients)
						{
							client->packetman.sendPacket<RootUpdatePacket>(PacketType::RootUpdate, packet, true);
						}
					}
				}
				queueUpdateRootIds.clear();
			}
			lastUpdateTime = se::time::now();
		}
	}

	void updateRoot(Root& _root)
	{
		if (_root.children.empty() && se::time::timeSince(_root.spawnTime) > growthInterval)
		{
			Root child;
			child.rootId = nextRootId.value++;
			child.start = _root.end;
			child.end = _root.end + glm::normalize(-_root.end) * rootLength;
			_root.children.push_back(child);
			sendRootCreate(child, &_root);
		}
		for (Root& child : _root.children)
		{
			updateRoot(child);
		}
	}

	void sendRootCreate(Root& _root, const Root *const parent)
	{
		RootCreatePacket packet;
		packet.start = _root.start;
		packet.end = _root.end;
		packet.parentRootId = parent ? parent->rootId : RootId();
		packet.health = _root.health;
		for (const std::unique_ptr<Client>& client : context.clients)
		{
			client->packetman.sendPacket<RootCreatePacket>(PacketType::RootCreate, packet, true);
		}
	}

	ServerContext& context;
	const float worldRadius;
	std::vector<Root> roots;
	std::unordered_set<RootId> queueUpdateRootIds;
	RootId nextRootId = RootId(1);
	se::time::Time lastSpawnTime;
	se::time::Time lastUpdateTime;
	se::ScopedConnections scopedConnections;
};

RootServer::RootServer(ServerContext& _context, float _worldSize)
	: impl(new Impl(_context, _worldSize))
{
}

RootServer::~RootServer()
{
	// ~Impl()
}

void RootServer::update()
{
	impl->update();
}

const std::vector<RootServer::Root>& RootServer::getRoots() const
{
	return impl->roots;
}

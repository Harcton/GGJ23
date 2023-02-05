#include "stdafx.h"
#include "Base/Server/RootServer.h"

#include "SpehsEngine/Graphics/Animator.h"
#include "SpehsEngine/Graphics/Model.h"
#include "SpehsEngine/Graphics/ModelDataManager.h"
#include "SpehsEngine/Graphics/ShaderManager.h"
#include "SpehsEngine/Graphics/Shape.h"
#include "SpehsEngine/Graphics/TextureManager.h"
#include "Base/ClientUtility/MaterialManager.h"
#include "Base/Server/Client.h"
#include "glm/gtx/rotate_vector.hpp"
#pragma optimize("", off)
using namespace se::graphics;


constexpr se::time::Time growthInterval = se::time::fromSeconds(15.0f);

struct RootServer::Impl
{
	RootServer::Impl(ServerContext& _context, float _worldSize)
		: context(_context)
		, worldRadius(_worldSize * 0.5f)
		, lastSpawnTime(se::time::now())
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

	void getChildChainDepth(int& depth, std::vector<Root>& _roots)
	{
		if (_roots.size() > 0)
		{
			getChildChainDepth(depth, _roots[0].children);
		}
		depth++;
	}
	bool rootDamageRecursive(std::vector<Root>& _roots, const RootDamagePacket& _packet)
	{
		for (size_t i = 0; i < _roots.size(); i++)
		{
			if (_roots[i].rootId == _packet.rootId)
			{
				int childChainDepth = -1;
				getChildChainDepth(childChainDepth, _roots[i].children);

				float baseDamage = _packet.damage;
				constexpr float rootArmorPiercing = 0.0f;
				const float childArmorFactor =
					  (1.0f / (float)pow(2.0f, childChainDepth))
					* (1.0f - rootArmorPiercing);

				_roots[i].health -= baseDamage * childArmorFactor;
				if (_roots[i].health <= 0.0f)
				{
					context.money += 1;
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
		if (!gameEnded && se::time::timeSince(startTime) > constants::gameWinTime)
		{
			sendGameEnd(true);
		}

		constexpr se::time::Time spawnInterval = se::time::fromSeconds(5.0f);
		if (se::time::timeSince(lastSpawnTime) > spawnInterval)
		{
			const float length = constants::defaultRootLength * se::rng::random(0.4f, 2.0f);

			Root root;
			root.rootId = nextRootId.value++;
			root.start = se::rng::circle(worldRadius);
			root.end = root.start + glm::normalize(-root.start) * length;
			root.health = 100.0f;
			root.spawnTime = se::time::now();
			root.rootStrain = RootStrain(se::rng::random<size_t>(0, size_t(RootStrain::Size) - 1));
			roots.push_back(root);

			sendRootCreate(root, nullptr);

			lastSpawnTime = se::time::now();
		}
		for (Root& root : roots)
		{
			updateRoot(root);
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
		if (_root.children.empty() && se::time::timeSince(_root.spawnTime) > growthInterval && se::time::timeSince(_root.childTime) > growthInterval)
		{
			const int numRoots = se::rng::weightedCoin(0.15f) ? 2 : 1;

			for (size_t i = 0; i < numRoots; i++)
			{
				const float length = constants::defaultRootLength * se::rng::random(0.4f, 2.0f);
				Root child;
				child.rootId = nextRootId.value++;
				child.health = 100.0f;
				child.start = _root.end;
				const glm::vec2 dir = glm::rotate(glm::normalize(-child.start),
					se::rng::random(-se::PI<float> *0.25f, se::PI<float> *0.25f));
				child.end = child.start + dir * length;
				child.spawnTime = se::time::now();
				child.rootStrain = _root.rootStrain;
				_root.childTime = se::time::now();
				_root.children.push_back(child);
				sendRootCreate(child, &_root);

				if (!gameEnded && (glm::distance(child.end, glm::vec2{}) < (constants::coreSize * 0.5)))
				{
					sendGameEnd(false);
				}
			}
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
		packet.rootId = _root.rootId;
		packet.parentRootId = parent ? parent->rootId : RootId();
		packet.health = _root.health;
		packet.rootStrain = _root.rootStrain;
		for (const std::unique_ptr<Client>& client : context.clients)
		{
			client->packetman.sendPacket<RootCreatePacket>(PacketType::RootCreate, packet, true);
		}
	}

	void forEachRoot(const std::function<void(const Root&)>& _func)
	{
		forEachRoot(_func, roots);
	}

	void apply(const RootDamagePacket& _packet)
	{
		rootDamageRecursive(roots, _packet);
	}

	void forEachRoot(const std::function<void(const Root&)>& _func, const std::vector<Root>& _roots) const
	{
		for (const Root& root : _roots)
		{
			_func(root);
			forEachRoot(_func, root.children);
		}
	}

	void sendGameEnd(bool win)
	{
		se_assert(!gameEnded);
		gameEnded = true;
		GameEndPacket packet;
		packet.win = win;

		for (const std::unique_ptr<Client>& client : context.clients)
		{
			client->packetman.sendPacket<GameEndPacket>(PacketType::GameEnd, packet, true);
		}
	}

	se::time::Time startTime = se::time::now();
	bool gameEnded = false;
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

void RootServer::apply(const RootDamagePacket& _packet)
{
	impl->apply(_packet);
}

void RootServer::forEachRoot(const std::function<void(const Root&)>& _func)
{
	impl->forEachRoot(_func);
}

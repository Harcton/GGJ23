#pragma once

struct RootDamagePacket;


class RootServer
{
public:

	struct Root
	{
		RootId rootId;
		RootStrain rootStrain = RootStrain::Blue;
		glm::vec2 start;
		glm::vec2 end;
		float health = 0.0f;
		se::time::Time spawnTime;
		se::time::Time childTime;
		std::vector<Root> children;
	};

	RootServer(ServerContext& _context, float _worldSize);
	~RootServer();
	void update();
	const std::vector<Root>& getRoots() const;
	void apply(const RootDamagePacket& packet);
	void forEachRoot(const std::function<void(const Root&)>& _func);
private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};


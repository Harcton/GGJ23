#pragma once



class RootServer
{
public:

	struct Root
	{
		RootId rootId;
		glm::vec2 start;
		glm::vec2 end;
		float health = 0.0f;
		se::time::Time spawnTime;
		std::vector<Root> children;
	};

	RootServer(ServerContext& _context, float _worldSize);
	~RootServer();
	void update();
	const std::vector<Root>& getRoots() const;
private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};


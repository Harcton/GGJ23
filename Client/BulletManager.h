#pragma once


struct DemoContext;

class BulletManager
{
public:
	BulletManager(DemoContext& _context, float _worldSize);
	~BulletManager();
	void update();
	void shoot(const glm::vec3& _pos, const glm::vec3& _dir);
	bool hitTest(const glm::vec3& _pos, float _radius);
private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};


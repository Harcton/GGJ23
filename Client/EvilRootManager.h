#pragma once


class BulletManager;

class EvilRootManager
{
public:
	EvilRootManager(ClientContext& _context, BulletManager& _bulletManager, float _worldSize);
	~EvilRootManager();
	void update();
private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};


#pragma once


struct DemoContext;
class BulletManager;

class PlayerCharacter
{
public:
	PlayerCharacter(DemoContext& _context, BulletManager& _bulletManager);
	~PlayerCharacter();
	void update();
private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};


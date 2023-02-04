#pragma once


class BulletManager;

class PlayerCharacter
{
public:
	PlayerCharacter(ClientContext& _context, BulletManager& _bulletManager);
	~PlayerCharacter();
	void update();
private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};


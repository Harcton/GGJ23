#pragma once


struct PlayerAttributes
{
	float movementSpeed = 30.0f;
	float weaponDamage = 20.0f;
	float weaponRange = 100.0f;
	float weaponRate = 1.0f;
	float weaponVelocity = 100.0f;
	float weaponSpread = 0.0f; // Radians, used only if shot size is greater than 1
	uint8_t weaponShotSize = 1;
};

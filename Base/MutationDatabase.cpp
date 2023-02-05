#include "stdafx.h"
#include "Base/MutationDatabase.h"

#include "Base/PlayerAttributes.h"
#pragma optimize("", off)

MutationDatabase::MutationDatabase()
{
	static const PlayerAttributes defaultPlayerAttributes;
	MutationId nextMutationId(1);

	auto addUpgrade = [&](const std::string_view _name, const uint16_t _maxStacks, const unsigned _cost, const std::function<void(PlayerAttributes&, const uint16_t)>& _function)
	{
		const MutationId mutationId = nextMutationId.value++;
		std::unique_ptr<Mutation>& mutation = lookup[mutationId];
		se_assert(!mutation);
		mutation.reset(new Mutation());
		mutation->mutationId = mutationId;
		mutation->maxStacks = _maxStacks;
		mutation->name = _name;
		mutation->cost = _cost;
		mutation->function = _function;
		mutation->mutationCategory = MutationCategory::Default;
	};
	auto addLoadout = [&](const std::string_view _name, const RootStrain _rootStrain, const std::function<void(PlayerAttributes&, const uint16_t)>& _function)
	{
		const MutationId mutationId = nextMutationId.value++;
		std::unique_ptr<Mutation>& mutation = lookup[mutationId];
		se_assert(!mutation);
		mutation.reset(new Mutation());
		mutation->mutationId = mutationId;
		mutation->maxStacks = 1;
		mutation->name = _name;
		mutation->function = _function;
		mutation->mutationCategory = MutationCategory::Loadout;
		mutation->rootStrain.emplace(_rootStrain);
	};

	addUpgrade("Anti wood bullets", 1000, 10,
		[&](PlayerAttributes& playerAttributes, const uint16_t stacks)
		{
			playerAttributes.weaponDamage *= 1.1f;
		});
	addUpgrade("Self adapt track", 1000, 10,
		[&](PlayerAttributes& playerAttributes, const uint16_t stacks)
		{
			playerAttributes.movementSpeed *= 1.1f;
		});

	addLoadout("#2E5894 - Ultra-Blue", RootStrain::Blue,
		[](PlayerAttributes& playerAttributes, const uint16_t)
		{
			playerAttributes.rootStrainLoadout = RootStrain::Blue;
			playerAttributes.weaponDamage *= 2.5f;
			playerAttributes.weaponVelocity *= 2.0f;
			playerAttributes.weaponRate *= 0.5f;
		});
	addLoadout("#8806CE - Ultra-Pink", RootStrain::Pink,
		[](PlayerAttributes& playerAttributes, const uint16_t)
		{
			playerAttributes.rootStrainLoadout = RootStrain::Pink;
			playerAttributes.weaponShotSize += 2;
			playerAttributes.weaponDamage *= 1.0f / 3.0f;
			playerAttributes.weaponSpread += se::PI<float> * 0.2f;
		});
	addLoadout("#CE2029 - Ultra-Red", RootStrain::Red,
		[](PlayerAttributes& playerAttributes, const uint16_t)
		{
			playerAttributes.rootStrainLoadout = RootStrain::Red;
			playerAttributes.weaponShotSize += 4;
			playerAttributes.weaponDamage *= 1.0f / 3.0f;
			playerAttributes.weaponRange *= 0.5f;
			playerAttributes.weaponSpread += se::PI<float> * 0.4f;
		});
	addLoadout("#FFEF00 - Ultra-Yellow", RootStrain::Yellow,
		[](PlayerAttributes& playerAttributes, const uint16_t)
		{
			playerAttributes.rootStrainLoadout = RootStrain::Yellow;
			playerAttributes.weaponDamage *= 0.25f;
			playerAttributes.weaponRate *= 4.0f;
		});

	vector.reserve(lookup.size());
	for (const std::pair<const MutationId, std::unique_ptr<Mutation>>& pair : lookup)
	{
		vector.push_back(pair.second.get());
	}
}

#include "stdafx.h"
#include "Base/MutationDatabase.h"

#include "Base/PlayerAttributes.h"
#pragma optimize("", off)

MutationDatabase::MutationDatabase()
{
	static const PlayerAttributes defaultPlayerAttributes;
	MutationId nextMutationId(1);

	auto addMutation = [&](const std::string_view _name, const uint16_t _maxStacks, MutationCategory _mutationCategory, const std::function<void(PlayerAttributes&, const uint16_t)>& _function)
	{
		const MutationId mutationId = nextMutationId.value++;
		std::unique_ptr<Mutation>& mutation = lookup[mutationId];
		se_assert(!mutation);
		mutation.reset(new Mutation());
		mutation->mutationId = mutationId;
		mutation->maxStacks = _maxStacks;
		mutation->name = _name;
		mutation->function = _function;
		mutation->mutationCategory = _mutationCategory;
	};

	addMutation("Minor speed buff", 5, MutationCategory::Default,
		[&](PlayerAttributes& playerAttributes, const uint16_t stacks)
		{
			playerAttributes.movementSpeed += float(stacks) * defaultPlayerAttributes.movementSpeed * 0.2f;
		});

	addMutation("Major speed buff", 1, MutationCategory::Default,
		[&](PlayerAttributes& playerAttributes, const uint16_t stacks)
		{
			playerAttributes.movementSpeed += float(stacks) * defaultPlayerAttributes.movementSpeed * 1.0f;
		});

	addMutation("Shot size upgrade", 10, MutationCategory::Default,
		[&](PlayerAttributes& playerAttributes, const uint16_t stacks)
		{
			playerAttributes.weaponShotSize += stacks;
			playerAttributes.weaponSpread += se::PI<float> * 0.05f * float(stacks);
		});

	vector.reserve(lookup.size());
	for (const std::pair<const MutationId, std::unique_ptr<Mutation>>& pair : lookup)
	{
		vector.push_back(pair.second.get());
	}
}
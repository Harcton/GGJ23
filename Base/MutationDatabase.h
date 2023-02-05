#pragma once

#include "Base/RootStrain.h"

struct PlayerAttributes;


struct Mutation
{
	MutationId mutationId;
	std::string name;
	uint16_t maxStacks = 1;
	unsigned cost = 0;
	std::optional<RootStrain> rootStrain;
	MutationCategory mutationCategory = MutationCategory::Default;
	std::function<void(PlayerAttributes&, const uint16_t)> function;
};

struct MutationDatabase
{
	MutationDatabase();

	const Mutation* find(const MutationId _mutationId) const
	{
		if (const std::unique_ptr<Mutation>* const mutation = tryFind(lookup, _mutationId))
		{
			return mutation->get();
		}
		else
		{
			return nullptr;
		}
	}
	uint16_t getMaxStacks(const MutationId _mutationId) const
	{
		if (const Mutation* const mutation = find(_mutationId))
		{
			return mutation->maxStacks;
		}
		else
		{
			return 0;
		}
	}

	std::unordered_map<MutationId, std::unique_ptr<Mutation>> lookup;
	std::vector<const Mutation*> vector;
};

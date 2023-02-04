#pragma once


enum class MutationCategory : uint8_t
{
	Default,
	Loadout, // Overwrites previous, only one can be active at a time
};

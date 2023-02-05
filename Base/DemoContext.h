#pragma once

#include "Base/EngineContext.h"

struct MutationDatabase;
class MaterialManager;
class UserSettings;
class SoundPlayer;
class GlobalHud;


struct DemoContext : public EngineContext
{
	MutationDatabase& mutationDatabase;
	UserSettings& userSettings;
	MaterialManager& materialManager;
	SoundPlayer& soundPlayer;
	GlobalHud& globalHud;
};
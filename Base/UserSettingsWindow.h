#pragma once

#include "Base/UserSettings.h"


class UserSettingsWindow
{
public:

	UserSettingsWindow(DemoContext &demoContext);

	void toggle() { windowOpen = !windowOpen; }

private:

	struct DisplayMode
	{
		int width = 0;
		int height = 0;
		std::string string;
	};
	void update();

	DemoContext& context;
	bool windowOpen = false;
	size_t displayModeIndex = 0;
	std::vector<DisplayMode> displayModes;
	se::ScopedConnections scopedConnections;
};

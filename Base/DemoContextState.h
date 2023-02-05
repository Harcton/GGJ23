#pragma once

#include "Base/DemoContext.h"


class DemoContextState
{
public:
	DemoContextState(const std::string_view _windowName, const std::string_view _processFilepath);
	~DemoContextState();

	void reset();
	bool update();
	void render();

	DemoContext getDemoContext();

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

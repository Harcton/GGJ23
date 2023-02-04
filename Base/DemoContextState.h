#pragma once

#include "Base/DemoContext.h"


class DemoContextState
{
public:
	DemoContextState(const std::string_view _windowName);
	~DemoContextState();

	void reset();
	bool update();
	void render();

	DemoContext getDemoContext();

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};

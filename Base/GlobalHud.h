#pragma once

class UserSettingsWindow;


class GlobalHud
{
public:
	GlobalHud(EngineContext& _context, UserSettingsWindow& _userSettingsWindow);
	~GlobalHud();
	void update();
private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};
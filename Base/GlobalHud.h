#pragma once

class UserSettingsWindow;


class GlobalHud
{
public:
	GlobalHud(EngineContext& _context, UserSettingsWindow& _userSettingsWindow);
	~GlobalHud();
	void update();
	bool getBackPressed();
	void setBackEnabled(const bool _enabled);
private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};
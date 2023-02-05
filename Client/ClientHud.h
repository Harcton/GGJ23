#pragma once

class UserSettingsWindow;


class ClientHud
{
public:
	ClientHud(DemoContext& _context, UserSettingsWindow& _userSettingsWindow);
	~ClientHud();
	void update();
private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};
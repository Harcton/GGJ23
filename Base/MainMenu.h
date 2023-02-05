#pragma once


enum class MainMenuResult : uint8_t { Server, Client };

class MainMenu
{
public:
	MainMenu(DemoContext& _context);
	~MainMenu();
	std::optional<MainMenuResult> getResult() const;
private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};
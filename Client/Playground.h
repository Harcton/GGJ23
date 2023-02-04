#pragma once


class Playground
{
public:
	Playground(ClientContext& _context);
	~Playground();
	void update();
private:
	class Impl;
	std::unique_ptr<Impl> impl;
};

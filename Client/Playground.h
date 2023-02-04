#pragma once


struct SessionContext;

class Playground
{
public:
	Playground(SessionContext& _context);
	~Playground();
	void update();
private:
	class Impl;
	std::unique_ptr<Impl> impl;
};

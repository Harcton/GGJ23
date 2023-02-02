#pragma once


struct DemoContext;

class Playground
{
public:
	Playground(DemoContext& _context);
	~Playground();
	void update();
private:
	class Impl;
	std::unique_ptr<Impl> impl;
};

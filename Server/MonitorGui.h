#pragma once


class MonitorGui : public IOperatorGui
{
public:

	MonitorGui(ServerContext& _context);
	~MonitorGui();

	std::optional<OperatorGui> update() final;

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};
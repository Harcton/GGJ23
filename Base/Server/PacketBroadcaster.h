#pragma once



class PacketBroadcaster
{
public:
	PacketBroadcaster(ServerContext& _context);
	~PacketBroadcaster();

private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};
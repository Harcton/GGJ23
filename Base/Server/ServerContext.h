#pragma once

#include "Base/DemoContext.h"
#include "Base/Server/Client.h"


struct ServerContext : public DemoContext
{
	std::vector<std::unique_ptr<Client>>& clients;
	unsigned money = 0;
};

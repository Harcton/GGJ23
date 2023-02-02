#pragma once

namespace se
{
	namespace net
	{
		class Connection2;
	}
}


struct Client
{
	ClientId clientId;
	std::string name;
	std::shared_ptr<se::net::Connection2> connection;
};

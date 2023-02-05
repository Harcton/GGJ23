#include "stdafx.h"

#include "Base/CombinedGame.h"


int main(const int argc, const char** argv)
{
	se_assert(argc > 0);
	const std::string processFilepath = argv[0];
	CombinedGame combinedGame(processFilepath, false);
	return 0;
}

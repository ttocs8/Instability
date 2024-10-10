#include "GlobalHelpers.hpp"


std::string GlobalHelpers::GetOSSeparator()
{
	std::string platform = SDL_GetPlatform();
	if (platform.find(WINDOWS) != std::string::npos) {
		return SEPARATOR_WIN;
	}
	else if (platform.find(LINUX) != std::string::npos)
	{
		return SEPARATOR_LNX;
	}
	else if (platform.find(MACOSX) != std::string::npos)
	{
		return SEPARATOR_LNX;
	}
	return SEPARATOR_WIN;
}

bool GlobalHelpers::FileExists(const std::string& Filename)
{
	return access(Filename.c_str(), 0) == 0;
}

#pragma once

enum class RootStrain : uint8_t
{
	Blue,
	Red,
	Pink,
	Yellow,
	Size,
};

static se::Color toColor(const RootStrain _rootStrain)
{
	switch (_rootStrain)
	{
	case RootStrain::Pink: return se::Color(0.99f, 0.42f, 0.62f);
	case RootStrain::Blue: return se::Color(0.18f, 0.35f, 0.58f);
	case RootStrain::Red: return se::Color(0.81f, 0.13f, 0.16f);
	case RootStrain::Yellow: return se::Color(1.0f, 0.94f, 0.0f);
	case RootStrain::Size: break;
	}
	return se::Color(1.0f, 1.0f, 1.0f);
}

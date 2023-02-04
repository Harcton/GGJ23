#pragma once


enum class RootStrain : uint8_t
{
	Green,
	Blue,
	Violet,
	Red,
	Orange,
	Pink,
	Yellow,
	Lime,
	Cyan,
	Size,
};

static se::Color toColor(const RootStrain _rootStrain)
{
	switch (_rootStrain)
	{
	case RootStrain::Green: return se::Color(0.29f, 0.44f, 0.27f);
	case RootStrain::Blue: return se::Color(0.18f, 0.35f, 0.58f);
	case RootStrain::Violet: return se::Color(0.53f, 0.02f, 0.81f);
	case RootStrain::Red: return se::Color(0.81f, 0.13f, 0.16f);
	case RootStrain::Orange: return se::Color(0.93f, 0.53f, 0.18f);
	case RootStrain::Pink: return se::Color(0.99f, 0.42f, 0.62f);
	case RootStrain::Yellow: return se::Color(1.0f, 0.94f, 0.0f);
	case RootStrain::Lime: return se::Color(0.62f, 0.99f, 0.22f);
	case RootStrain::Cyan: return se::Color(0.0f, 1.0f, 1.0f);
	case RootStrain::Size: break;
	}
	return se::Color(1.0f, 1.0f, 1.0f);
}


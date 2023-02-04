#include "stdafx.h"
#include "Base/ClientUtility/SoundPlayer.h"

#include "SpehsEngine/Audio/AudioEngine.h"
#include "SpehsEngine/Audio/AudioManager.h"
#include "SpehsEngine/Audio/AudioResource.h"
#include "SpehsEngine/Audio/AudioSource.h"


using namespace se::audio;

SoundPlayer::SoundPlayer(se::audio::AudioManager& _audioManager, se::audio::AudioEngine& _audioEngine)
	: audioManager(_audioManager)
	, audioEngine(_audioEngine)
{}
SoundPlayer::~SoundPlayer()
{}

std::shared_ptr<AudioResource> SoundPlayer::getResource(std::string_view _audioFile)
{
	std::shared_ptr<AudioResource> result = audioManager.find(_audioFile);
	if (!result)
	{
		result = audioManager.load(_audioFile, _audioFile);
	}
	result->waitUntilReady(); // :`(
	return result;
}

void SoundPlayer::init()
{
	musicBus = std::make_unique<Bus>();
	musicBus->connect(audioEngine.getMasterBus());
}
void SoundPlayer::update()
{
	for (auto it = sources.begin(); it != sources.end();)
	{
		if (!it->second->isPlaying() && !it->second->isPaused())
		{
			it = sources.erase(it);
		}
		else
		{
			it++;
		}
	}
}
float SoundPlayer::getMasterVolume() const
{
	return audioEngine.getMasterBus().getVolume();
}
float SoundPlayer::getMusicVolume() const
{
	return musicBus->getVolume();
}
void SoundPlayer::setMasterVolume(float _value)
{
	audioEngine.getMasterBus().setVolume(_value);
}
void SoundPlayer::setMusicVolume(float _value)
{
	musicBus->setVolume(_value);
}

SoundId SoundPlayer::playMusic(std::string_view _audioFile, se::time::Time _fade, int _layer)
{
	auto resource = getResource(_audioFile);
	auto& source = sources[++soundId];
	source = std::make_unique<AudioSource>();
	source->setResource(resource);
	source->setProtected(true);
	source->setLooping(true);
	source->setOutput(*musicBus);
	source->playBackground();
	if (_fade != se::time::Time::zero)
	{
		source->setVolume(0.0f);
		source->setVolume(1.0f, _fade);
	}

	auto mit = musicSources.find(_layer);
	if (mit != musicSources.end())
	{
		auto it = sources.find(mit->second);
		if (it != sources.end())
		{
			if (_fade != se::time::Time::zero)
			{
				it->second->setLooping(false); // not good, but just to make sure sound is cleanedup...
				it->second->setVolume(0.0f, _fade);
			}
			else
			{
				sources.erase(it);
			}
		}
	}
	musicSources[_layer] = soundId;

	return soundId;
}
SoundId SoundPlayer::playSound(std::string_view _audioFile, const glm::vec3& _position)
{
	auto resource = getResource(_audioFile);
	auto& source = sources[++soundId];
	source = std::make_unique<AudioSource>();
	source->setResource(resource);
	source->setPosition(_position);
	source->setOutput(audioEngine.getMasterBus());
	source->play();
	return soundId;
}
void SoundPlayer::stopSound(SoundId _id)
{
	sources.erase(_id);
}
void SoundPlayer::setSoundPosition(SoundId _id, const glm::vec3& _position)
{
	auto it = sources.find(_id);
	if (it == sources.end())
		return;

	for (auto&& music : musicSources)
		se_assert(_id != music.second);

	it->second->setPosition(_position);
}
void SoundPlayer::setSoundLooping(SoundId _id, bool _looping)
{
	auto it = sources.find(_id);
	if (it == sources.end())
		return;

	for (auto&& music : musicSources)
		se_assert(_id != music.second);

	it->second->setLooping(_looping);
}

#pragma once

namespace se::audio
{
	class AudioManager;
	class AudioEngine;
	class AudioSource;
	class AudioResource;
	class Bus;
}
struct DemoContext;


using SoundId = uint32_t;

class SoundPlayer
{
public:

	SoundPlayer(se::audio::AudioManager& _audioManager, se::audio::AudioEngine& _audioEngine);
	~SoundPlayer();

	void init();
	void update();
	float getMasterVolume() const;
	float getMusicVolume() const;
	float getSfxVolume() const;
	void setMasterVolume(float _value);
	void setMusicVolume(float _value);
	void setSfxVolume(float _value);

	SoundId playMusic(std::string_view _audioFile, se::time::Time _fade = se::time::Time::zero, int _layer = 0);
	SoundId playSound(std::string_view _audioFile, const glm::vec3& _position);
	void stopSound(SoundId _id);
	void setSoundPosition(SoundId _id, const glm::vec3& _position);
	void setSoundLooping(SoundId _id, bool _looping);

private:

	std::shared_ptr<se::audio::AudioResource> getResource(std::string_view _audioFile);

	se::audio::AudioManager& audioManager;
	se::audio::AudioEngine& audioEngine;

	SoundId soundId = 0;
	std::unordered_map<SoundId, std::unique_ptr<se::audio::AudioSource>> sources;
	std::unordered_map<int, SoundId> musicSources;
	std::unique_ptr<se::audio::Bus> musicBus;
	std::unique_ptr<se::audio::Bus> sfxBus;
};

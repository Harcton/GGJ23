#pragma once

namespace se::audio
{
	class AudioSource;
	class AudioResource;
	class Bus;
}
struct DemoContext;


using SoundId = uint32_t;

class SoundPlayer
{
public:

	SoundPlayer(DemoContext& _context);
	~SoundPlayer();

	void update();
	float getMasterVolume() const;
	float getMusicVolume() const;
	void setMasterVolume(float _value);
	void setMusicVolume(float _value);

	SoundId playMusic(std::string_view _audioFile, se::time::Time _fade = se::time::Time::zero, int _layer = 0);
	SoundId playSound(std::string_view _audioFile, const glm::vec3& _position);
	void stopSound(SoundId _id);
	void setSoundPosition(SoundId _id, const glm::vec3& _position);
	void setSoundLooping(SoundId _id, bool _looping);

private:

	std::shared_ptr<se::audio::AudioResource> getResource(std::string_view _audioFile);

	DemoContext& context;

	SoundId soundId = 0;
	std::unordered_map<SoundId, std::unique_ptr<se::audio::AudioSource>> sources;
	std::unordered_map<int, SoundId> musicSources;
	std::unique_ptr<se::audio::Bus> musicBus;
};

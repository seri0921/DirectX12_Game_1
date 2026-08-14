#pragma once

#include <Windows.h>
#include <xaudio2.h>
#include "GameUtil.h"

class SoundSource
{
public:
	SoundSource(IXAudio2* xaudio2, SoundData* soundData, bool start = true,
		UINT32 loopCount = 0, float volume = 1.0f);
	~SoundSource();

	bool isValid() { return m_valid; }

	void start();
	void pause();
	void stop();

	void update(float deltaTime);
	void setVolume(float volume);

	enum class SoundState
	{
		Play, Pause, Stop
	};
	SoundState getState() const { return m_state; }

private:
	IXAudio2SourceVoice* m_sourceVoice;
	bool m_valid;
	SoundState m_state;


};
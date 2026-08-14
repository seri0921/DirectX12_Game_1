#include "SoundSource.h"

SoundSource::SoundSource(IXAudio2* xaudio2, SoundData* soundData, bool start,
	UINT32 loopCount, float volume)
	: m_sourceVoice(nullptr)
	, m_valid(false)
	, m_state(SoundState::Stop)
{
	HRESULT hr = xaudio2->CreateSourceVoice(&m_sourceVoice, &(soundData->format));
	if (FAILED(hr)) return;
	m_valid = true;

	XAUDIO2_BUFFER buf = {};
	buf.pAudioData = soundData->waveData.data();
	buf.Flags = XAUDIO2_END_OF_STREAM;
	buf.LoopCount = loopCount;
	buf.AudioBytes = sizeof(BYTE) * (uint32_t)soundData->waveData.size();
	m_sourceVoice->SubmitSourceBuffer(&buf);
	m_sourceVoice->SetVolume(volume);
	if (start)
	{
		m_sourceVoice->Start();
		m_state = SoundState::Play;
	}
	else
	{
		m_state = SoundState::Pause;
	}
}

SoundSource::~SoundSource()
{
	if (m_sourceVoice != nullptr)
	{
		m_sourceVoice->Stop();
		m_sourceVoice->DestroyVoice();
		m_sourceVoice = nullptr;
		m_valid = false;
	}
}

void SoundSource::start()
{
	if (!m_valid) return;
	if (m_state == SoundState::Pause)
	{
		m_sourceVoice->Start();
		m_state = SoundState::Play;
	}
}

void SoundSource::pause()
{
	if (!m_valid) return;
	if (m_state == SoundState::Play)
	{
		m_sourceVoice->Stop();
		m_state = SoundState::Pause;
	}
}

void SoundSource::stop()
{
	if (!m_valid) return;
	if (m_state == SoundState::Play)
	{
		m_sourceVoice->Stop();
		m_state = SoundState::Stop;
	}
}

void SoundSource::update(float deltaTime)
{
	if (!m_valid) return;
	if (m_state == SoundState::Play)
	{
		XAUDIO2_VOICE_STATE state = {};
		m_sourceVoice->GetState(&state);
		if (state.BuffersQueued == 0)
		{
			m_state = SoundState::Stop;
		}
	}
}

void SoundSource::setVolume(float volume)
{
	if (!m_valid) return;

	m_sourceVoice->SetVolume(volume);
}
#pragma once

#include <Windows.h>
#include <xaudio2.h>

#include <wrl.h>
using namespace Microsoft::WRL;

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include "GameUtil.h"
#include "SoundSource.h"

class SoundSystem
{
public:
	SoundSystem();
	~SoundSystem();

	bool initialize();
	SoundInfo loadSoundFile(std::wstring filePath);
	void releaseSoundFile(SoundInfo sinfo);

	bool setBGM(std::wstring filePath, bool start = false);
	bool startBGM();
	bool pauseBGM();
	bool stopBGM();
	bool setSoundEffect(std::wstring filePath);

	void update(float deltaTime);
	void setBGMVolume(float volume);
	void setSEVolume(float volume);

private:
	static const UINT SoundDataMaxNum = 1000;
	static const UINT SoundEffectMaxNum = 20;

	ComPtr<IXAudio2> m_xaudio2;
	IXAudio2MasteringVoice* m_masteringVoice;
	std::unordered_map<std::wstring, int> m_soundDataList;
	std::unique_ptr<SoundData> m_soundData[SoundDataMaxNum];
	int m_soundDataReferenceCount[SoundDataMaxNum];

	SoundSource* m_bgm;
	SoundSource* m_soundEffects[SoundEffectMaxNum];
	float m_bgmVolume;
	float m_seVolume;
	SoundInfo m_bgmInfo;
	SoundInfo m_seInfo[SoundEffectMaxNum];
};


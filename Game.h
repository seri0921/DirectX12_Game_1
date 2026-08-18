#pragma once

#include <Windows.h>
#include "GameUtil.h"
#include "Keyboard.h"
#include "Mouse.h"

#include <memory>
#include "Scene.h"
#include "Renderer.h"
#include "SoundSystem.h"
#include <random>

class Game
{
public:
	Game();
	~Game();

	void initialize(HWND hwnd, int width, int height);
	bool loop();

	HWND getHwnd() { return m_hwnd; }
	int getWidth() const { return m_width; }
	int getHeight() const { return m_height; }

	const Keyboard& getKeyboard() const { return m_keyboard; }
	const Mouse& getMouse() const { return m_mouse; }
	Renderer* getRenderer() { return m_renderer.get(); }
	SoundSystem* getSoundSystem() { return m_soundSystem.get(); }

	double getRand();
	int getRand(int minValue, int maxValue);
	bool getBoolRand();


private:
	HWND m_hwnd;
	int m_width;
	int m_height;

	bool m_isRunning;

	static const float FrameRate;
	static const float MaxDeltaTime;
	LARGE_INTEGER m_startTime;
	LARGE_INTEGER m_endTime;
	LARGE_INTEGER m_freqTime;

	Keyboard m_keyboard;
	Mouse m_mouse;

	std::vector<Scene*> m_scene;
	std::unique_ptr<Renderer> m_renderer;
	std::unique_ptr<SoundSystem> m_soundSystem;

	std::mt19937_64 m_rand;
	std::uniform_real_distribution<double> m_uniRand;

	void input();
	void update(float deltaTime);
	void draw();

	bool tick(float& deltaTime);
};
#pragma once

#include <Windows.h>
#include "GameUtil.h"
#include "Keyboard.h"

class Game
{
public:
	Game();
	~Game();

	void initialize(HWND hwnd, int width, int height);
	bool loop();

	void drawCircle(const Circle& circle, COLORREF color);

private:
	HWND m_hwnd;
	int m_width;
	int m_height;

	bool m_isRunning;

	HBITMAP m_backBuffer;
	HDC m_backBufferDC;
	COLORREF m_backColor;

	Circle m_ball;
	Vector2d m_ballVel;
	Circle m_player;
	float m_playerSpeed;

	static const float FrameRate;
	static const float MaxDeltaTime;
	LARGE_INTEGER m_startTime;
	LARGE_INTEGER m_endTime;
	LARGE_INTEGER m_freqTime;

	Keyboard m_keyboard;

	void input();
	void update(float deltaTime);
	void draw();

	void clear();
	void flip();

	bool tick(float& deltaTime);
};
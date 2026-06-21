#include "Game.h"
#include <exception>

const float Game::FrameRate = 60.0f;
const float Game::MaxDeltaTime = 0.05f;

Game::Game()
	: m_hwnd(nullptr)
	, m_width(0)
	, m_height(0)
	, m_isRunning(false)
	, m_backBuffer(nullptr)
	, m_backBufferDC(nullptr)
	, m_backColor(RGB(255, 255, 255))
	, m_startTime{}
	, m_endTime{}
	, m_freqTime{}
	, m_ball()
	, m_ballVel()
	, m_player()
	, m_playerSpeed()
{
}

Game::~Game()
{
	{
		// バックバッファの解放
		if (m_backBufferDC)
		{
			DeleteDC(m_backBufferDC);
			m_backBufferDC = nullptr;
		}
		if (m_backBuffer)
		{
			DeleteObject(m_backBuffer);
			m_backBuffer = nullptr;
		}
	}
}

void Game::initialize(HWND hwnd, int width, int height)
{
	if (width <= 0 || height <= 0) throw std::exception();

	m_hwnd = hwnd;
	m_width = width;
	m_height = height;

	{
		// バックバッファの初期化
		HDC hdc = GetDC(hwnd);
		m_backBuffer = CreateCompatibleBitmap(hdc, m_width, m_height);
		m_backBufferDC = CreateCompatibleDC(hdc);
		SelectObject(m_backBufferDC, m_backBuffer);
		ReleaseDC(hwnd, hdc);
	}

	{
		// ボールの初期化
		m_ball = Circle(Vector2d(50.0f, 200.0f), 10.0f);
		m_ballVel = Vector2d(250.0f, 150.0f);

		// プレイヤーの初期化
		m_player = Circle(Vector2d(320.0f, 240.0f), 60.0f);
		m_playerSpeed = 100.0f;
	}

	// 時間計測の初期化
	QueryPerformanceFrequency(&m_freqTime);
	QueryPerformanceCounter(&m_startTime);

	// キーボードの初期化
	m_keyboard.initialize();

	// ゲーム状態の初期化
	m_isRunning = true;
}

bool Game::loop()
{
	float deltaTime = 0.0f;
	if (tick(deltaTime))
	{
		input();
		update(deltaTime);
		if (m_isRunning == false) return false;
		draw();
	}
	
	return m_isRunning;
}

void Game::input()
{
	// キーボードやゲームパッドなどの入力を受け取る処理を記述
	m_keyboard.input();
}

void Game::update(float deltaTime)
{
	{
		// プレイヤーの更新
		int vx = 0, vy = 0;
		if (m_keyboard.isDown(VK_LEFT)) vx -= 1;
		if (m_keyboard.isDown(VK_RIGHT)) vx += 1;
		if (m_keyboard.isDown(VK_UP)) vy -= 1;
		if (m_keyboard.isDown(VK_DOWN)) vy += 1;
		Vector2d playerVel = m_playerSpeed * normalize(Vector2d((float) vx, (float) vy));

		m_player.pos += deltaTime * playerVel;

		float r = m_player.radius;
		if (m_player.pos.x - r < 0.0f) m_player.pos.x = r;
		if (m_player.pos.x + r >= m_width) m_player.pos.x = m_width - r;
		if (m_player.pos.y - r < 0.0f) m_player.pos.y = r;
		if (m_player.pos.y + r >= m_height) m_player.pos.y = m_height - r;
	}

	// ゲームの状態を更新する処理を記述
	{
		// ボールの移動
		m_ball.pos += m_ballVel * deltaTime;

		float r = m_ball.radius;
		if (m_ball.pos.x - r < 0.0f)
		{
			m_ball.pos.x = r;
			m_ballVel.x *= -1.0f;
		}
		if (m_ball.pos.x + r >= m_width)
		{
			m_ball.pos.x = m_width - r;
			m_ballVel.x *= -1.0f;
		}
		if (m_ball.pos.y - r < 0.0f)
		{
			m_ball.pos.y = r;
			m_ballVel *= -1.0f;
		}
		if (m_ball.pos.y + r >= m_height)
		{
			m_ball.pos.y = m_height - r;
			m_ballVel.y *= -1.0f;
		}
	}

	// 衝突判定
	if (detectCircleCollision(m_player, m_ball))
	{
		m_isRunning = false;
		return;
	}
}
	
void Game::draw()
{
	clear();

	// ゲームの状態を描画する処理を記述
	drawCircle(m_player, RGB(30, 30, 255));
	drawCircle(m_ball, RGB(255, 30, 30));

	flip();
}

void Game::clear()
{
	HBRUSH backBrush = CreateSolidBrush(m_backColor);
	HBRUSH oldBrush = (HBRUSH)SelectObject(m_backBufferDC, backBrush);
	PatBlt(m_backBufferDC, 0, 0, m_width, m_height, PATCOPY);
	SelectObject(m_backBufferDC, oldBrush);
	DeleteObject(backBrush);
}

void Game::flip()
{
	HDC hdc = GetDC(m_hwnd);
	BitBlt(hdc, 0, 0, m_width, m_height, m_backBufferDC, 0, 0, SRCCOPY);
	ReleaseDC(m_hwnd, hdc);
}

void Game::drawCircle(const Circle& circle, COLORREF color)
{
	int left = (int)(std::round(circle.pos.x - circle.radius));
	int top = (int)(std::round(circle.pos.y - circle.radius));
	int right = (int)(std::round(circle.pos.x + circle.radius));
	int bottom = (int)(std::round(circle.pos.y + circle.radius));

	HBRUSH circleBrush = CreateSolidBrush(color);
	HBRUSH oldBrush = (HBRUSH)SelectObject(m_backBufferDC, circleBrush);
	Ellipse(m_backBufferDC, left, top, right, bottom);
	SelectObject(m_backBufferDC, oldBrush);
	DeleteObject(circleBrush);
}

bool Game::tick(float& deltaTime)
{
	QueryPerformanceCounter(&m_endTime);
	if (m_endTime.QuadPart - m_startTime.QuadPart == 0) return false;

	deltaTime = (float)(m_endTime.QuadPart - m_startTime.QuadPart)
		/ (float)m_freqTime.QuadPart;
	if (deltaTime < (1.0f / (FrameRate + 1.0f))) return false;

	m_startTime = m_endTime;
	deltaTime = (deltaTime > MaxDeltaTime) ? MaxDeltaTime : deltaTime;
	return true;
}
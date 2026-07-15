#include "Game.h"
#include <exception>
#include "TestScene.h"

const float Game::FrameRate = 60.0f;
const float Game::MaxDeltaTime = 0.05f;

Game::Game()
	: m_hwnd(nullptr)
	, m_width(0)
	, m_height(0)
	, m_isRunning(false)
	, m_startTime{}
	, m_endTime{}
	, m_freqTime{}
{
}

Game::~Game()
{
	// シーンの解放
	if (m_scene.get() != nullptr)
	{
		m_scene.reset();
	}

}

void Game::initialize(HWND hwnd, int width, int height)
{
	if (width <= 0 || height <= 0) throw std::exception();

	m_hwnd = hwnd;
	m_width = width;
	m_height = height;

	// レンダラーの初期化
	m_renderer = std::make_unique<Renderer>(this, 0.0f, 0.0f, 0.0f);
	if (!m_renderer->initialize()) throw std::exception();

	// 時間計測の初期化
	QueryPerformanceFrequency(&m_freqTime);
	QueryPerformanceCounter(&m_startTime);

	// キーボードの初期化
	m_keyboard.initialize();

	// シーンの初期化
	m_scene = std::make_unique<TestScene>(this);

}

bool Game::loop()
{
	float deltaTime = 0.0f;
	if (tick(deltaTime))
	{
		input();
		update(deltaTime);
		if (m_scene->isRunning() == false) return false;
		draw();

		//int dt = (int)std::round(deltaTime * 1000.0f);
		//printNum(L"delta time = %d[msec]\n", dt);
	}
	
	return m_scene->isRunning();
}

void Game::input()
{
	// キーボードやゲームパッドなどの入力を受け取る処理を記述
	m_keyboard.input();
}

void Game::update(float deltaTime){
	m_scene->update(deltaTime);
}
	
void Game::draw()
{
	m_renderer->begin();

	m_scene->draw();

	m_renderer->end();
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
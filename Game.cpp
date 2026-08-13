#include "Game.h"
#include <exception>
#include "TestScene.h"
#include "ShootingScene.h"

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
	, m_uniRand(0.0, 1.0)
{
	// 乱数初期化
	std::random_device seed;
	m_rand = std::mt19937_64(seed());
}

Game::~Game()
{
	// シーンの解放
	if (m_scene.get() != nullptr)
	{
		m_scene.reset();
	}

	// レンダラーの解放
	m_renderer.reset();

	// COMの終了
	CoUninitialize();

}

void Game::initialize(HWND hwnd, int width, int height)
{
	if (width <= 0 || height <= 0) throw std::exception();

	m_hwnd = hwnd;
	m_width = width;
	m_height = height;

	// COMの初期化
	{
		HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
		if (FAILED(hr)) throw std::exception();
	}

	// レンダラーの初期化
	m_renderer = std::make_unique<Renderer>(this, ColorBlack);
	if (!m_renderer->initialize()) throw std::exception();

	// 時間計測の初期化
	QueryPerformanceFrequency(&m_freqTime);
	QueryPerformanceCounter(&m_startTime);

	// キーボードの初期化
	m_keyboard.initialize();
	// マウスの初期化
	m_mouse.initialize(m_hwnd);

	// シーンの初期化
	m_scene = std::make_unique<ShootingScene>(this);

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
	// キーボードやゲームパッドなどの入力
	m_keyboard.input();
	m_mouse.input();
}

void Game::update(float deltaTime){
	m_renderer->update(deltaTime);

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

double Game::getRand()
{
	// m_uniRandを使って一様乱数（0.0以上、1.0未満）を生成
	return m_uniRand(m_rand);
}

int Game::getRand(int minValue, int maxValue)
{
	// minValue以上、maxValue以下の一様乱数を整数型（int型）で生成
	// 乱数値の幅（range）の計算ではmaxValueを含めるために1加算している
	// m_uniRandの一様乱数は1.0未満であることに注意
	// 例： 1～6のサイコロの目を作るなら getRand（1, 6） とする

	double range = (double)(maxValue - minValue + 1);
	return minValue + (int)(range * m_uniRand(m_rand));
}

bool Game::getBoolRand()
{
	//getRand（0, 1）で0か1のどちらかの値の一様乱数となるので、これをtrue/falseに変換して返す

	return (getRand() < 0.5) ? true : false;
}

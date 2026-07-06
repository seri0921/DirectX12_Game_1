#include "Game.h"
#include <exception>
#include "BreakoutScene.h"

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
{
}

Game::~Game()
{
	// シーンの解放
	if (m_scene.get() != nullptr)
	{
		m_scene.reset();
	}

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

	{
		// 画像データの削除
		for (auto it = m_spriteImages.begin(); it != m_spriteImages.end(); ++it)
		{
			releaseImageData(it->second);
		}
		m_spriteImages.clear();
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

	// 時間計測の初期化
	QueryPerformanceFrequency(&m_freqTime);
	QueryPerformanceCounter(&m_startTime);

	// キーボードの初期化
	m_keyboard.initialize();

	// シーンの初期化
	m_scene = std::make_unique<BreakoutScene>(this);

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
	clear();

	m_scene->draw();
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

void Game::drawString(const wchar_t* str, const Vector2d& pos,
	COLORREF color, int fsize)
{
	int x = (int)(std::round(pos.x));
	int y = (int)(std::round(pos.y));
	int len = (int)wcslen(str);

	int oldMode = GetBkMode(m_backBufferDC);
	SetBkMode(m_backBufferDC, TRANSPARENT);
	COLORREF oldColor = SetTextColor(m_backBufferDC, color);

	HFONT font = CreateFontW(fsize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		FF_DONTCARE, nullptr);
	HFONT oldFont = (HFONT)SelectObject(m_backBufferDC, font);

	TextOutW(m_backBufferDC, x, y, str, len);

	SelectObject(m_backBufferDC, oldFont);
	SetBkMode(m_backBufferDC, oldMode);
	SetTextColor(m_backBufferDC, oldColor);
	DeleteObject(font);
}

void Game::drawString(const wchar_t* str, int num, const Vector2d& pos,
	COLORREF color, int fsize)
{
	static wchar_t buf[1024];
	wsprintfW(buf, str, num);

	drawString(buf, pos, color, fsize);
}

void Game::drawRect(Box rect, COLORREF fillColor, COLORREF penColor)
{
	int left = (int)std::round(rect.pos.x);
	int top = (int)std::round(rect.pos.y);
	int right = (int)std::round(rect.pos.x + rect.width);
	int bottom = (int)std::round(rect.pos.y + rect.height);

	HBRUSH rectBrush = CreateSolidBrush(fillColor);
	HBRUSH oldBrush = (HBRUSH)SelectObject(m_backBufferDC, rectBrush);
	HPEN rectPen = CreatePen(PS_SOLID, 1, penColor);
	HPEN oldPen = (HPEN)SelectObject(m_backBufferDC, rectPen);

	Rectangle(m_backBufferDC, left, top, right, bottom);

	SelectObject(m_backBufferDC, oldBrush);
	SelectObject(m_backBufferDC, oldPen);
	DeleteObject(rectBrush);
	DeleteObject(rectPen);
}

bool Game::loadSprite(const std::wstring& filePath, int& imgWidth, int& imgHeight)
{
	if (m_spriteImages.find(filePath) != m_spriteImages.end())
	{
		ImageData img = m_spriteImages[filePath];
		imgWidth = img.width;
		imgHeight = img.height;
		return true;
	}

	ImageData img;
	if (loadImageData(filePath, img))
	{
		m_spriteImages[filePath] = img;
		imgWidth = img.width;
		imgHeight = img.height;
		return true;
	}
	return false;
}

bool Game::drawSprite(const std::wstring& filePath,
	const Vector2d& pos, const Vector2d& offset) 
{
	auto it = m_spriteImages.find(filePath);
	if (it == m_spriteImages.end())
	{
		int w, h;
		if (!loadSprite(filePath, w, h)) return false;
		it = m_spriteImages.find(filePath);
	}

	ImageData sprite = it->second;
	Vector2d p = pos + offset;
	int x = (int)std::round(p.x);
	int y = (int)std::round(p.y);
	HDC hdcImg = CreateCompatibleDC(m_backBufferDC);
	SelectObject(hdcImg, sprite.img);
	BitBlt(m_backBufferDC, x, y, sprite.width, sprite.height, hdcImg,
		0, 0, SRCCOPY);
	DeleteDC(hdcImg);

	return true;
}
#include "Mouse.h"

Mouse::Mouse()
	: m_hwnd(nullptr)
	, m_keyStates{ 0 }
	, m_keyOldStates{ 0 }
	, m_pos(ZeroVec2d)
{
}

Mouse::~Mouse()
{
}

void Mouse::initialize(HWND hwnd)
{
	m_hwnd = hwnd;
	memset(m_keyStates, 0, KeyNum * sizeof(SHORT));
	memset(m_keyOldStates, 0, KeyNum * sizeof(SHORT));
}

void Mouse::input()
{
	memcpy(m_keyOldStates, m_keyStates, KeyNum * sizeof(SHORT));

	for (int i = 0; i < KeyNum; ++i)
	{
		m_keyStates[i] = GetAsyncKeyState(VK_Keys[i]);
	}

	POINT pos;
	GetCursorPos(&pos);
	ScreenToClient(m_hwnd, &pos);
	m_pos = XMFLOAT2((float)pos.x, (float)pos.y);
}

bool Mouse::keyIndex(BYTE key, int& index) const
{
	for (index = 0; index < KeyNum; ++index)
	{
		if (key == VK_Keys[index]) break;
	}
	if (index >= KeyNum) return false;

	return true;
}

bool Mouse::isPressed(BYTE key) const
{
	int index = 0;
	if (!keyIndex(key, index)) return false;

	if (m_keyOldStates[index] & 0x8000) return false;
	if (m_keyStates[index] & 0x8000) return true;
	return false;
}

bool Mouse::isReleased(BYTE key) const
{
	if (m_keyStates[key] & 0x8000) return false;
	if (m_keyOldStates[key] & 0x8000) return true;
	return false;
}

bool Mouse::isDown(BYTE key) const
{
	int index = 0;
	if (!keyIndex(key, index)) return false;

	if (m_keyStates[index] & 0x8000) return true;
	return false;
}

bool Mouse::isUp(BYTE key) const
{
	int index = 0;
	if (!keyIndex(key, index)) return false;

	if (m_keyStates[index] & 0x8000) return false;
	return true;
}
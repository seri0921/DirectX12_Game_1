#pragma once

#include <Windows.h>
#include "GameUtil.h"

class Mouse
{
public:
	Mouse();
	~Mouse();

	void initialize(HWND hwnd);
	void input();

	bool isPressed(BYTE key) const;
	bool isReleased(BYTE key) const;
	bool isDown(BYTE key) const;
	bool isUp(BYTE key) const;

	XMFLOAT2 getPos() const { return m_pos; }

private:
	static constexpr int KeyNum = 3;
	static constexpr BYTE VK_Keys[KeyNum] =
	{ VK_LBUTTON, VK_RBUTTON, VK_MBUTTON };
	HWND m_hwnd;
	XMFLOAT2 m_pos;

	SHORT m_keyStates[KeyNum];
	SHORT m_keyOldStates[KeyNum];

	bool keyIndex(BYTE key, int& index) const;
};


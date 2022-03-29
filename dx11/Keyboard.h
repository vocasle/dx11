#pragma once

#include <stdint.h>
#include <vector>

#include <Windows.h>

enum Keys
{
	Keys_W = 'W',
	Keys_A = 'A',
	Keys_S = 'S',
	Keys_D = 'D',
	Keys_Up = VK_UP,
	Keys_Left = VK_LEFT,
	Keys_Down = VK_DOWN,
	Keys_Right = VK_RIGHT,
	Keys_Plus = VK_OEM_PLUS,
	Keys_Minus = VK_OEM_MINUS,
	Keys_Num = 10
};

struct Keyboard
{
	Keyboard();
	std::vector<WPARAM> Keys;
	std::vector<uint32_t> States;
};

void KeyboardOnKeyDown(struct Keyboard* keyboard, WPARAM wParam);
void KeyboardOnKeyUp(struct Keyboard* keyboard, WPARAM wParam);

uint32_t KeyboardIsKeyDown(const struct Keyboard* keyboard, WPARAM key);
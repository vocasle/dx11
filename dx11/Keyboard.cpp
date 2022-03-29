#include "Keyboard.h"

void KeyboardInit(struct Keyboard* keyboard)
{	
	keyboard->Keys[0] = Keys_W;
	keyboard->Keys[1] = Keys_A;
	keyboard->Keys[2] = Keys_S;
	keyboard->Keys[3] = Keys_D;
	keyboard->Keys[4] = Keys_Up;
	keyboard->Keys[5] = Keys_Left;
	keyboard->Keys[6] = Keys_Down;
	keyboard->Keys[7] = Keys_Right;
	keyboard->Keys[8] = Keys_Plus;
	keyboard->Keys[9] = Keys_Minus;
}

void KeyboardOnKeyDown(struct Keyboard* keyboard, WPARAM wParam)
{
	for (uint32_t i = 0; i < Keys_Num; ++i)
	{
		if (keyboard->Keys[i] == wParam)
		{
			keyboard->States[i] = 1;
		}
	}
}

void KeyboardOnKeyUp(struct Keyboard* keyboard, WPARAM wParam)
{
	for (uint32_t i = 0; i < Keys_Num; ++i)
	{
		if (keyboard->Keys[i] == wParam)
		{
			keyboard->States[i] = 0;
		}
	}
}

uint32_t KeyboardIsKeyDown(const struct Keyboard* keyboard, WPARAM key)
{
	for (uint32_t i = 0; i < Keys_Num; ++i)
	{
		if (keyboard->Keys[i] == key)
		{
			return keyboard->States[i];
		}
	}
	return 0;
}

Keyboard::Keyboard():
	Keys(Keys_Num),
	States(Keys_Num)
{
	KeyboardInit(this);
}

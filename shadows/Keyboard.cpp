#include "Keyboard.h"

void KeyboardInit(struct Keyboard* keyboard)
{
	keyboard->Keys = new WPARAM[Keys_Num];
	keyboard->States = new uint32_t[Keys_Num];
	WPARAM* keys = keyboard->Keys;
	keys[0] = Keys_W;
	keys[1] = Keys_A;
	keys[2] = Keys_S;
	keys[3] = Keys_D;
	keys[4] = Keys_Up;
	keys[5] = Keys_Left;
	keys[6] = Keys_Down;
	keys[7] = Keys_Right;
	keys[8] = Keys_Plus;
	keys[9] = Keys_Minus;
}

void KeyboardDeinit(struct Keyboard* keyboard)
{
	free(keyboard->Keys);
	free(keyboard->States);
	keyboard->Keys = NULL;
	keyboard->States = NULL;
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

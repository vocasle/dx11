#include "Mouse.h"
#include "Utils.h"

#include <string.h>
#include <windowsx.h>

void MouseInit(struct Mouse* mouse, uint32_t width, uint32_t height)
{
	memset(mouse, 0, sizeof(struct Mouse));
	mouse->WinSize.X = width;
	mouse->WinSize.Y = height;
	ShowCursor(0);
}

Vec2D MouseGetCursorPos(const struct Mouse* mouse)
{
	return mouse->MousePos;
}

void MouseOnMouseMove(struct Mouse* mouse, uint32_t message, WPARAM wParam, LPARAM lParam)
{
	static uint32_t firstLaunch = 1;
	mouse->MousePos.X = GET_X_LPARAM(lParam);
	mouse->MousePos.Y = GET_Y_LPARAM(lParam);
	POINT pos = {};
	GetCursorPos(&pos);
	mouse->MousePos.X = pos.x;
	mouse->MousePos.Y = pos.y;
	SetCursorPos((int32_t)mouse->WinSize.X / 2, (int32_t)mouse->WinSize.Y / 2);
	if (firstLaunch)
	{
		firstLaunch = 0;
		mouse->MousePos.X = mouse->WinSize.X / 2;
		mouse->MousePos.Y = mouse->WinSize.Y / 2;
	}
}

Vec2D MouseGetMouseDelta(const struct Mouse* mouse)
{

	Vec2D delta = {0};
	delta.X = mouse->WinSize.X / 2.0f - mouse->MousePos.X;
	delta.Y = mouse->WinSize.Y / 2.0f - mouse->MousePos.Y;

	//UtilsDebugPrint("delta={%f %f}\n", delta.X, delta.Y);

	return delta;
}

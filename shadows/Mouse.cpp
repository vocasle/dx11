#include "Mouse.h"
#include "Utils.h"

#include <string.h>
#include <windowsx.h>

std::unique_ptr<Mouse> Mouse::m_Instance = nullptr;

Mouse &Mouse::Get()
{
	if (!m_Instance) {
		m_Instance = std::unique_ptr<Mouse>(new Mouse());
	}
	return *m_Instance;
}

Mouse::Mouse()
{
}

Mouse::~Mouse()
{
}

void Mouse::SetWindowDimensions(uint32_t width, uint32_t height)
{
	WinSize.X = width;
	WinSize.Y = height;
	ShowCursor(0);
}

Vec2D Mouse::GetCursorPos()
{
	return MousePos;
}

void Mouse::OnMouseMove(uint32_t message, WPARAM wParam, LPARAM lParam)
{
	static uint32_t firstLaunch = 1;
	MousePos.X = GET_X_LPARAM(lParam);
	MousePos.Y = GET_Y_LPARAM(lParam);
	POINT pos = {};
	::GetCursorPos(&pos);
	MousePos.X = pos.x;
	MousePos.Y = pos.y;
	SetCursorPos((int32_t)WinSize.X / 2, (int32_t)WinSize.Y / 2);
	if (firstLaunch) {
		firstLaunch = 0;
		MousePos.X = WinSize.X / 2;
		MousePos.Y = WinSize.Y / 2;
	}
}

Vec2D Mouse::GetMouseDelta()
{
	Vec2D delta = {};
	delta.X = WinSize.X / 2.0f - MousePos.X;
	delta.Y = WinSize.Y / 2.0f - MousePos.Y;

	// UtilsDebugPrint("delta={%f %f}\n", delta.X, delta.Y);

	return delta;
}

#pragma once
#include "Math.h"

#include <stdint.h>

#include "Windows.h"

struct Mouse
{
	Mouse();
	Vec2D MousePos;
	Vec2D WinSize;
};

void MouseInit(struct Mouse* mouse, uint32_t width, uint32_t height);

Vec2D MouseGetCursorPos(const struct Mouse* mouse);

void MouseOnMouseMove(struct Mouse* mouse, uint32_t message, WPARAM wParam, LPARAM lParam);

Vec2D MouseGetMouseDelta(const struct Mouse* mouse);

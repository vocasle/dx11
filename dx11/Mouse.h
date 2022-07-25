#pragma once
#include "Math.h"

#include <stdint.h>

#include "Windows.h"

struct Mouse
{
  uint32_t LeftBtnState;
  uint32_t RightBtnState;
  Vec2D MousePos;
  uint32_t Mode;
  Vec2D WinSize;
};

void MouseInit (struct Mouse *mouse, uint32_t width, uint32_t height);

Vec2D MouseGetCursorPos (const struct Mouse *mouse);

void MouseOnMouseMove (struct Mouse *mouse, uint32_t message, WPARAM wParam,
                       LPARAM lParam);

void MouseOnToggleFullscreen (struct Mouse *mouse,
                              const uint32_t isFullscreen);

Vec2D MouseGetMouseDelta (const struct Mouse *mouse);

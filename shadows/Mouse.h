#pragma once
#include "Math.h"

#include <cstdint>
#include <memory>

#include "Windows.h"

class Mouse {
    public:
	static Mouse &Get();
	~Mouse();

	void SetWindowDimensions(uint32_t width, uint32_t height);
	Vec2D GetCursorPos();
	void OnMouseMove(uint32_t message, WPARAM wParam, LPARAM lParam);
	Vec2D GetMouseDelta();

    private:
	Mouse();

	static std::unique_ptr<Mouse> m_Instance;

	uint32_t LeftBtnState;
	uint32_t RightBtnState;
	Vec2D MousePos;
	uint32_t Mode;
	Vec2D WinSize;
};

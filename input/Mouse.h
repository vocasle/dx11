#pragma once
#include "NE_Math.h"

#include <cstdint>
#include <memory>

#include "Windows.h"

class Mouse {
    public:
	enum class ButtonType { Left, Scroll, Right };

    public:
	static Mouse &Get();
	~Mouse();

	void SetWindowDimensions(uint32_t width, uint32_t height);
	Vec2D GetCursorPos();
	void OnMouseMove(uint32_t message, WPARAM wParam, LPARAM lParam);
	void OnMouseDown(uint32_t message, WPARAM wParam, LPARAM lParam,
			 ButtonType type);
	void OnMouseUp(uint32_t message, WPARAM wParam, LPARAM lParam,
		       ButtonType type);
	Vec2D GetMouseDelta();

    private:
	Mouse();

	static std::unique_ptr<Mouse> m_Instance;

	bool m_LeftBtnState;
	bool m_RightBtnState;
	Vec2D MousePos;
	Vec2D WinSize;
};

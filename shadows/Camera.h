#pragma once

#include "Math.h"
#include "Keyboard.h"
#include "Mouse.h"

#include <stdint.h>

#define CAMERA_SENSITIVITY 0.01f
#define MOUSE_SENSITIVITY 0.0001f

class Camera
{
public:
	Camera(const Vec3D& cameraPos,
		const Keyboard* keyboard,
		const Mouse* mouse);
	Mat4X4 GetViewMat() const;
	void UpdatePos(double deltaMillis);
	void ProcessMouse(double deltaMillis);
	Vec3D GetPos() const { return m_Pos; }
	Vec3D GetAt() const { return m_At; }

private:
	void UpdateVectors();
	void UpdateSpeed();

	float m_Pitch;
	float m_Yaw;
	Vec3D m_Pos;
	Vec3D m_At;
	Vec3D m_Right;
	Vec3D m_Up;
	const Keyboard* m_Keyboard;
	const Mouse* m_Mouse;
	float m_Speed;
};
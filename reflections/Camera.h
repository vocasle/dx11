#pragma once

#include "Math.h"

#include <stdint.h>

#define CAMERA_SENSITIVITY 0.01f
#define MOUSE_SENSITIVITY 0.0001f

class Camera {
    public:
	Camera();
	Camera(const Vec3D &cameraPos);
	Mat4X4 GetViewMat() const;
	Mat4X4 GetProjMat() const;
	void UpdatePos(double deltaMillis);
	void ProcessMouse(double deltaMillis);
	Vec3D GetPos() const
	{
		return m_Pos;
	}
	Vec3D GetAt() const
	{
		return m_At;
	}
	void SetViewDimensions(uint32_t width, uint32_t height);
	void SetZNear(const float zNear);
	void SetZFar(const float zFar);
	void LookAt(const Vec3D &pos, const Vec3D &target, const Vec3D &up);
	void SetFov(float fov);

    private:
	void UpdateVectors();
	void UpdateSpeed();

	float m_Pitch;
	float m_Yaw;
	Vec3D m_Pos;
	Vec3D m_At;
	Vec3D m_Right;
	Vec3D m_Up;
	float m_Speed;
	float m_backBufferWidth;
	float m_backBufferHeight;
	float m_zNear;
	float m_zFar;
	float m_fov = 45.0f;
};
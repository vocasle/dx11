#pragma once

#include "Keyboard.h"
#include "Math.h"
#include "Mouse.h"

#include <stdint.h>

#define CAMERA_SENSITIVITY 0.01f
#define MOUSE_SENSITIVITY 0.0001f

struct Camera {
	float Pitch;
	float Yaw;
	Vec3D CameraPos;
	Vec3D FocusPoint;
	Vec3D Right;
	Vec3D Up;
	const struct Keyboard *Keyboard;
	const struct Mouse *Mouse;
	float Speed;
};

void CameraInit(struct Camera *camera, const Vec3D *cameraPos,
		const struct Keyboard *keyboard, const struct Mouse *mouse);

Mat4X4 CameraGetViewMat(struct Camera *camera);

void CameraUpdatePos(struct Camera *camera, double deltaMillis);

void CameraProcessMouse(struct Camera *camera, double deltaMillis);
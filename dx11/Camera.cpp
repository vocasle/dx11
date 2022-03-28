#include "Camera.h"
#include "Utils.h"

#include <corecrt_math_defines.h>
#include <math.h>

static void CameraUpdateVectors(struct Camera* camera);

void CameraInit(struct Camera* camera,
	const Vec3D* cameraPos,
	const struct Keyboard* keyboard,
	const struct Mouse* mouse)
{
	memset(camera, 0, sizeof(struct Camera));
	camera->CameraPos = *cameraPos;
	camera->Pitch = 0.0f;
	camera->Yaw = (float)M_PI_2;
	camera->Keyboard = keyboard;
	camera->Mouse = mouse;
	camera->Speed = 1.0f;
	CameraUpdateVectors(camera);
}

Mat4X4 CameraGetViewMat(struct Camera* camera)
{
	const Vec3D direction = MathVec3DAddition(&camera->CameraPos, &camera->FocusPoint);
	return MathMat4X4ViewAt(&camera->CameraPos, &direction, &camera->Up);
}

static void CameraUpdateSpeed(struct Camera* camera)
{
	if (KeyboardIsKeyDown(camera->Keyboard, VK_OEM_PLUS))
	{
		camera->Speed += 0.5f;
	}
	else if (KeyboardIsKeyDown(camera->Keyboard, VK_OEM_MINUS))
	{
		camera->Speed -= 0.5f;
	}

	camera->Speed = MathClamp(0.1f, 100.0f, camera->Speed);
}

void CameraUpdatePos(struct Camera* camera, double deltaMillis)
{
	CameraUpdateSpeed(camera);
	Vec3D cameraFocus = camera->FocusPoint;
	const float delta = (float)deltaMillis * CAMERA_SENSITIVITY * camera->Speed;


	if (KeyboardIsKeyDown(camera->Keyboard, VK_LEFT) || KeyboardIsKeyDown(camera->Keyboard, 'A'))
	{
		Vec3D right = MathVec3DCross(&cameraFocus, &camera->Up);
		MathVec3DNormalize(&right);
		right = MathVec3DModulateByScalar(&right, delta);
		camera->CameraPos = MathVec3DAddition(&camera->CameraPos, &right);
	}
	else if (KeyboardIsKeyDown(camera->Keyboard, VK_RIGHT) || KeyboardIsKeyDown(camera->Keyboard, 'D'))
	{
		Vec3D right = MathVec3DCross(&cameraFocus, &camera->Up);
		MathVec3DNormalize(&right);
		right = MathVec3DModulateByScalar(&right, delta);
		camera->CameraPos = MathVec3DSubtraction(&camera->CameraPos, &right);
	}
	else if (KeyboardIsKeyDown(camera->Keyboard, VK_UP))
	{
		const Vec3D up = MathVec3DModulateByScalar(&camera->Up, delta);
		camera->CameraPos = MathVec3DAddition(&camera->CameraPos, &up);
	}
	else if (KeyboardIsKeyDown(camera->Keyboard, VK_DOWN))
	{
		const Vec3D up = MathVec3DModulateByScalar(&camera->Up, delta);
		camera->CameraPos = MathVec3DSubtraction(&camera->CameraPos, &up);
	}
	else if (KeyboardIsKeyDown(camera->Keyboard, 'W'))
	{
		cameraFocus = MathVec3DModulateByScalar(&cameraFocus, delta);
		camera->CameraPos = MathVec3DAddition(&camera->CameraPos, &cameraFocus);
	}
	else if (KeyboardIsKeyDown(camera->Keyboard, 'S'))
	{
		cameraFocus = MathVec3DModulateByScalar(&cameraFocus, delta);
		camera->CameraPos = MathVec3DSubtraction(&camera->CameraPos, &cameraFocus);
	}
}

static void CameraUpdateVectors(struct Camera* camera)
{
	const float d = cosf(camera->Pitch);
	const float x = d * cosf(camera->Yaw);
	const float z = d * sinf(camera->Yaw);
	const float y = sinf(camera->Pitch);

	Vec3D direction = { x, y, z };
	MathVec3DNormalize(&direction);

	camera->FocusPoint = direction;
	MathVec3DNormalize(&camera->FocusPoint);
	const Vec3D worldUp = { 0.0f, 1.0f, 0.0f };
	camera->Right = MathVec3DCross(&worldUp, &camera->FocusPoint);
	MathVec3DNormalize(&camera->Right);
	camera->Up = MathVec3DCross(&camera->FocusPoint, &camera->Right);
	MathVec3DNormalize(&camera->Up);
}

void CameraProcessMouse(struct Camera* camera, double deltaMillis)
{
	const Vec2D mouseDelta = MouseGetMouseDelta(camera->Mouse);

	static const float MAX_PITCH = (float)(M_PI_2 - 0.1);
	camera->Yaw += mouseDelta.X * MOUSE_SENSITIVITY * (float)deltaMillis;
	camera->Pitch += mouseDelta.Y * MOUSE_SENSITIVITY * (float)deltaMillis;
	camera->Pitch = MathClamp(-MAX_PITCH, MAX_PITCH, camera->Pitch);

	//static float xLastPos = 0;
	//static float yLastPos = 0;

	//if (xLastPos == 0.0f || yLastPos == 0.0f)
	//{
	//	xLastPos = camera->Mouse->MousePos.X;
	//	yLastPos = camera->Mouse->MousePos.Y;
	//	return;
	//}

	//const float xOffset = (camera->Mouse->MousePos.X - xLastPos) * MOUSE_SENSITIVITY * (float)deltaMillis;
	//const float yOffset = (yLastPos - camera->Mouse->MousePos.Y) * MOUSE_SENSITIVITY * (float)deltaMillis;

	//xLastPos = camera->Mouse->MousePos.X;
	//yLastPos = camera->Mouse->MousePos.Y;

	//static const float MAX_PITCH = (float)(M_PI_2 - 0.1);

	//camera->Yaw -= xOffset;
	//camera->Pitch += yOffset; // reverse y because in screen space y goes from top to bottom of the screen
	//camera->Pitch = MathClamp(-MAX_PITCH, MAX_PITCH, camera->Pitch);

	CameraUpdateVectors(camera);
}

#pragma once

#include <stdlib.h>
#include <stdint.h>

#include <profileapi.h>
#include <processthreadsapi.h>
#include <debugapi.h>

#define MAX_DELTA_TIME 1000

typedef struct Timer
{
	double DeltaMillis;
	LARGE_INTEGER StartTime;
	LARGE_INTEGER EndTime;
	LARGE_INTEGER Frequency;
	LARGE_INTEGER ElapsedMicroseconds;
} Timer;

inline void TimerInitialize(Timer* timer)
{
	timer->DeltaMillis = 0.0;
	memset(timer, 0, sizeof(Timer));
	QueryPerformanceFrequency(&timer->Frequency);
}

inline void TimerTick(Timer* timer)
{
	if (!QueryPerformanceCounter(&timer->EndTime))
	{
		OutputDebugStringA("ERROR: Failed to query performance counter\n");
		ExitProcess(EXIT_FAILURE);
	}
	
	uint64_t timeDelta = timer->EndTime.QuadPart - timer->StartTime.QuadPart;

	timeDelta *= 1000000;
	timeDelta /= timer->Frequency.QuadPart;

	timer->DeltaMillis = (double)timeDelta / 1000.0; // convert to ms
	if (timer->DeltaMillis > MAX_DELTA_TIME)
		timer->DeltaMillis = MAX_DELTA_TIME;
	timer->StartTime = timer->EndTime;
}
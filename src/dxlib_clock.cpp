

#include "dxlib_clock.h"

#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>

CDxLibClock::CDxLibClock()
{
	restart();
}

CDxLibClock::~CDxLibClock()
{

}

float CDxLibClock::getElapsedTime()
{
	unsigned long long nFrequency = DxLib::GetSysPerformanceFrequency();
	unsigned long long nNow = getNowCounter();
	return static_cast<float>(nNow - m_nLastCounter) / nFrequency;
}

void CDxLibClock::restart()
{
	m_nLastCounter = getNowCounter();
}

unsigned long long CDxLibClock::getNowCounter()
{
	return DxLib::GetNowSysPerformanceCount();
}

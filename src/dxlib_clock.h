﻿#ifndef DXLIB_CLOCK_H_
#define DXLIB_CLOCK_H_

class CDxLibClock
{
public:
	CDxLibClock();
	~CDxLibClock();

	float GetElapsedTime();
	void Restart();
private:
	unsigned long long m_nLastCounter{};

	unsigned long long GetNowCounter();
};

#endif // !DXLIB_CLOCK_H_

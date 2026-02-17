#ifndef DXLIB_CLOCK_H_
#define DXLIB_CLOCK_H_

class CDxLibClock
{
public:
	CDxLibClock();
	~CDxLibClock();

	float getElapsedTime();
	void restart();
private:
	unsigned long long m_nLastCounter = 0;

	unsigned long long getNowCounter();
};

#endif // !DXLIB_CLOCK_H_

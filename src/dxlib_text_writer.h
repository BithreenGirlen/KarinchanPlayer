#ifndef DXLIB_TEXT_WRITER_H_
#define DXLIB_TEXT_WRITER_H_

class CDxLibTextWriter
{
public:
	CDxLibTextWriter();
	~CDxLibTextWriter();

	bool setFont(const wchar_t* fontFileName, int iFontSize, bool bold, bool italic);
	void draw(const wchar_t* pText, unsigned long textLength, int iPosX = 0, int iPosY = 0) const;

	void toggleTextColour();
private:
	int m_iFontHandle = -1;

	enum Colours : unsigned int
	{
		kBlack = 0xff000000,
		kWhite = 0xffffffff,
	};

	unsigned int m_uiFillColour = Colours::kBlack;
	unsigned int m_uiOutLineColour = Colours::kWhite;

	bool deleteFont();
};
#endif // !DXLIB_TEXT_WRITER_H_

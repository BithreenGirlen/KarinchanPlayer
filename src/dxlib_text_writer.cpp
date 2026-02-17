

#include "dxlib_text_writer.h"

#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>

CDxLibTextWriter::CDxLibTextWriter()
{

}

CDxLibTextWriter::~CDxLibTextWriter()
{
    deleteFont();
}

bool CDxLibTextWriter::setFont(const wchar_t* fontFileName, int iFontSize, bool bold, bool italic)
{
    deleteFont();

    m_iFontHandle = DxLib::CreateFontToHandle
    (
        fontFileName,
        iFontSize > 0 ? iFontSize : DEFAULT_FONT_SIZE,
        bold ? DEFAULT_FONT_THICKNESS: DEFAULT_FONT_THICKNESS / 2,
        DX_FONTTYPE_ANTIALIASING_EDGE_4X4,
        DX_CHARSET_UTF8,
        DEFAULT_FONT_EDGESIZE,
        italic ? TRUE : FALSE
    );

    if (m_iFontHandle != -1)
    {
        DxLib::SetFontLineSpaceToHandle(static_cast<int>(iFontSize * 1.6), m_iFontHandle);
    }

    return m_iFontHandle != -1;
}

void CDxLibTextWriter::draw(const wchar_t* pText, unsigned long textLength, int iPosX, int iPosY) const
{
    if (m_iFontHandle != -1)
    {
        DxLib::DrawNStringToHandle(iPosX + 4, iPosY + 4, pText, textLength, m_uiFillColour, m_iFontHandle, m_uiOutLineColour);
    }
}

void CDxLibTextWriter::toggleTextColour()
{
    m_uiFillColour = m_uiFillColour == Colours::kBlack ? Colours::kWhite : Colours::kBlack;
    m_uiOutLineColour = m_uiOutLineColour == Colours::kBlack ? Colours::kWhite : Colours::kBlack;
}

bool CDxLibTextWriter::deleteFont()
{
    if (m_iFontHandle != -1)
    {
        int iRet = DxLib::DeleteFontToHandle(m_iFontHandle);
        if (iRet == -1)return false;

        m_iFontHandle = -1;
    }

    return true;
}

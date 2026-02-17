

#include "karinchan_scene_player.h"

#include "karinchan.h"
#include "win_text.h"


struct SDxLibRenderTarget
{
	SDxLibRenderTarget(int iGraphicHandle, bool toClear = true)
	{
		DxLib::SetDrawScreen(iGraphicHandle);
		if (toClear)DxLib::ClearDrawScreen();
	};
	~SDxLibRenderTarget()
	{
		DxLib::SetDrawScreen(DX_SCREEN_BACK);
	}
};


CKarinchanScenePlayer::CKarinchanScenePlayer()
{
	m_dxLibTextWriter.setFont(L"游明朝", 28, true, true);

	m_pAudioPlayer = std::make_unique<CMfMediaPlayer>();
}

CKarinchanScenePlayer::~CKarinchanScenePlayer()
{

}
/*台本読み込み*/
bool CKarinchanScenePlayer::loadScenario(const std::wstring& folderPath)
{
	clearScenarioData();

	std::vector<adv::ImageFileDatum> imageFileData;

	karinchan::ReadScenario(folderPath, m_textData, imageFileData, m_sceneData, m_labelData);

	bool hasAnimationBeenLoaded = false;
	for (const auto& imageFileDatum : imageFileData)
	{
		if (imageFileDatum.isAnimation)
		{
			if (!hasAnimationBeenLoaded)
			{
				std::vector<std::string> atlasPaths;
				std::vector<std::string> skelPaths;

				std::string atlasPath = win_text::NarrowUtf8(folderPath) + "\\animation.atlas.txt";
				std::string skelPath = win_text::NarrowUtf8(folderPath) + "\\animation.json";

				atlasPaths.push_back(std::move(atlasPath));
				skelPaths.push_back(std::move(skelPath));

				hasAnimationBeenLoaded = m_dxLibSpinePlayer.loadSpineFromFile(atlasPaths, skelPaths, false);
				if (hasAnimationBeenLoaded)
				{
					m_dxLibSpinePlayer.setOffset(0, 0);
					m_dxLibSpinePlayer.setBaseSize(kDefaultWidth, kDefaultHeight);
				}
			}

			SImageDatum imageDatum;
			imageDatum.isAnimation = true;
			imageDatum.animationParams.loop = imageFileDatum.animationParams.loop;
			imageDatum.animationParams.usIndex = imageFileDatum.animationParams.usIndex;

			m_imageData.push_back(std::move(imageDatum));
		}
		else
		{
			/* 通常1920 x 1080だが、1280 x 720のものが混じっているので、こちらは拡大して表示する。 */

			int iWidth = 0;
			int iHeight = 0;
			float fScale = 1.f;
			if (DxLib::GetImageSize_File(imageFileDatum.wstrFilePath.c_str(), &iWidth, &iHeight) != -1)
			{
				fScale = static_cast<float>(kDefaultWidth) / iWidth;
			}

			DxLibImageHandle dxLibImageHandle(DxLib::LoadGraph(imageFileDatum.wstrFilePath.c_str()));

			if (dxLibImageHandle.get() != -1)
			{
				m_imageHandles.push_back(std::move(dxLibImageHandle));

				SImageDatum imageDatum;
				imageDatum.isAnimation = false;
				imageDatum.stillParams.usIndex = static_cast<unsigned short>(m_imageHandles.size() - 1);
				imageDatum.stillParams.fScale = fScale;

				m_imageData.push_back(std::move(imageDatum));
			}
		}
	}

	workOutDefaultScale();
	resetScale();

	prepareScene();

	m_spineClock.restart();

	return !m_imageData.empty();
}

bool CKarinchanScenePlayer::hasScenarioData() const
{
	return !m_sceneData.empty();
}

void CKarinchanScenePlayer::update()
{
	float fDelta = m_spineClock.getElapsedTime();
	const auto* p = getCurrentImageDatum();
	if (p != nullptr)
	{
		if (p->isAnimation)
		{
			m_dxLibSpinePlayer.update(fDelta);
		}
	}

	checkTextClock();

	m_spineClock.restart();
}

void CKarinchanScenePlayer::draw()
{
	drawCurrentImage();
	drawFormattedText();
}

void CKarinchanScenePlayer::getStillImageSize(unsigned int* width, unsigned int* height) const
{
	if (width != nullptr)*width = static_cast<unsigned int>(kDefaultWidth * m_fScale);
	if (height != nullptr)*height = static_cast<unsigned int>(kDefaultHeight * m_fScale);
}
/*場面移行*/
void CKarinchanScenePlayer::shiftScene(bool forward)
{
	if (m_sceneData.empty())return;

	if (forward)
	{
		if (++m_nSceneIndex >= m_sceneData.size())
		{
			m_nSceneIndex = 0;
		}
	}
	else
	{
		if (--m_nSceneIndex >= m_sceneData.size())
		{
			m_nSceneIndex = m_sceneData.size() - 1;
		}
	}

	prepareScene();
}
/*最終場面是否*/
bool CKarinchanScenePlayer::hasReachedLastScene() const
{
	return m_nSceneIndex == m_sceneData.size() - 1;
}
/*文字色切り替え*/
void CKarinchanScenePlayer::toggleTextColour()
{
	m_dxLibTextWriter.toggleTextColour();
}
/*尺度変更*/
void CKarinchanScenePlayer::rescaleImage(bool upscale)
{
	if (upscale)
	{
		m_fScale += kScaleDelta;
	}
	else
	{
		m_fScale -= kScaleDelta;
		if (m_fScale < kMinScale)m_fScale = kMinScale;
	}

	const auto& skeletonSize = m_dxLibSpinePlayer.getBaseSize();
	float fSkeletonScaleToBe = kDefaultWidth * m_fScale / skeletonSize.x;
	fSkeletonScaleToBe *= kDefaultSkeletonScale;
	m_dxLibSpinePlayer.setSkeletonScale(fSkeletonScaleToBe);
}
/*時間尺度変更*/
void CKarinchanScenePlayer::rescaleAnimationTime(short scroll)
{
	static constexpr float kTimeScaleDelta = 0.05f;
	const float scrollSign = scroll > 0 ? 1.f : -1.f;

	float timeScale = m_dxLibSpinePlayer.getTimeScale() + kTimeScaleDelta * scrollSign;
	timeScale = (std::max)(timeScale, 0.f);
	m_dxLibSpinePlayer.setTimeScale(timeScale);
}
/*視点移動*/
void CKarinchanScenePlayer::addOffset(int iX, int iY)
{
	const auto* p = getCurrentImageDatum();
	if (p == nullptr)return;

	if (p->isAnimation)
	{
		m_dxLibSpinePlayer.addOffset(iX, iY);
	}
	else
	{
		m_fOffset.u += iX * m_fScale;
		m_fOffset.v += iY * m_fScale;

		const auto AdjustViewForStill = [this]()
			-> void
			{
				int iTargetWidth = 0;
				int iTargetHeight = 0;
				DxLib::GetDrawScreenSize(&iTargetWidth, &iTargetHeight);

				float fScaledWidth = kDefaultWidth * m_fScale;
				float fScaledHeight = kDefaultHeight * m_fScale;

				float fMaxOffsetX = fScaledWidth - iTargetWidth;
				float fMaxOffsetY = fScaledHeight - iTargetHeight;

				m_fOffset.u = (std::max)(-fMaxOffsetX, m_fOffset.u);
				m_fOffset.v = (std::max)(-fMaxOffsetY, m_fOffset.v);

				m_fOffset.u = (std::min)(fMaxOffsetX, m_fOffset.u);
				m_fOffset.v = (std::min)(fMaxOffsetY, m_fOffset.v);
			};

		AdjustViewForStill();
	}
}
/*尺度・位置初期化*/
void CKarinchanScenePlayer::resetScale()
{
	m_fScale = m_fDefaultScale;
	m_fOffset = {};

	resetSpinePlayerScale();
}

void CKarinchanScenePlayer::onResize(int width, int height)
{
	m_renderTexture = DxLib::MakeScreen(width, height, 1);
}

const std::vector<adv::LabelDatum>& CKarinchanScenePlayer::getLabelData() const
{
	return m_labelData;
}

bool CKarinchanScenePlayer::jumpToLabel(size_t nLabelIndex)
{
	if (nLabelIndex < m_labelData.size())
	{
		const auto& labelDatum = m_labelData[nLabelIndex];

		if (labelDatum.nSceneIndex < m_sceneData.size())
		{
			m_nSceneIndex = labelDatum.nSceneIndex;
			prepareScene();

			return true;
		}
	}
	return false;
}
/*台本データ消去*/
void CKarinchanScenePlayer::clearScenarioData()
{
	m_textData.clear();

	m_sceneData.clear();
	m_nSceneIndex = 0;

	m_imageData.clear();
	m_imageHandles.clear();

	m_wstrFormattedText.clear();
	m_lastAnimationIndex = 0;

	m_labelData.clear();
}
/*標準尺度算出*/
void CKarinchanScenePlayer::workOutDefaultScale()
{
	m_fDefaultScale = 1.f;

	/* 静止画の大きさが1920 x 1080なので、これよりディスプレイ解像度が小さい場合は縮小する。 */

	int iSceneWidth = kDefaultWidth;
	int iSceneHeight = kDefaultHeight;

	int iDisplayWidth = 0;
	int iDisplayHeight = 0;
#if defined _WIN32
	DxLib::GetDisplayMaxResolution(&iDisplayWidth, &iDisplayHeight);
#elif defined __ANDROID__
	DxLib::GetAndroidDisplayResolution(&iDisplayWidth, &iDisplayHeight);
#elif defined __APPLE__
	DxLib::GetDisplayResolution_iOS(&iDisplayWidth, &iDisplayHeight);
#endif
	if (iDisplayWidth == 0 || iDisplayHeight == 0)return;

	if (iSceneWidth > iDisplayWidth || iSceneHeight > iDisplayHeight)
	{
		float fScaleX = static_cast<float>(iDisplayWidth) / iSceneWidth;
		float fScaleY = static_cast<float>(iDisplayHeight) / iSceneHeight;

		if (fScaleX > fScaleY)
		{
			m_fDefaultScale = fScaleY;
		}
		else
		{
			m_fDefaultScale = fScaleX;
		}
	}
}
/*現在の画像データ取り出し*/
CKarinchanScenePlayer::SImageDatum* CKarinchanScenePlayer::getCurrentImageDatum()
{
	if (m_nSceneIndex < m_sceneData.size())
	{
		size_t nImageIndex = m_sceneData[m_nSceneIndex].nImageIndex;
		if (nImageIndex < m_imageData.size())
		{
			return &m_imageData[nImageIndex];
		}
	}
	return nullptr;
}
/*場面描画事前準備*/
void CKarinchanScenePlayer::prepareScene()
{
	prepareText();
	checkAnimationTrack();
}
/*動作切り替わり場面か確認*/
void CKarinchanScenePlayer::checkAnimationTrack()
{
	const auto* p = getCurrentImageDatum();
	if (p != nullptr)
	{
		if (p->isAnimation)
		{
			if (m_lastAnimationIndex != p->animationParams.usIndex)
			{
				m_lastAnimationIndex = p->animationParams.usIndex;
				m_dxLibSpinePlayer.setAnimationByIndex(m_lastAnimationIndex - 1ULL);
			}
		}
		else
		{
			m_lastAnimationIndex = 0;
		}
	}
}
/*文章作成・附随音声再生*/
void CKarinchanScenePlayer::prepareText()
{
	if (m_nSceneIndex < m_sceneData.size())
	{
		size_t nTextIndex = m_sceneData[m_nSceneIndex].nTextIndex;
		if (nTextIndex < m_textData.size())
		{
			m_wstrFormattedText.clear();
			std::wstring& wstr = m_wstrFormattedText;
			const auto& t = m_textData[nTextIndex];

			wstr += t.wstrName + L"\n" + t.wstrText;
			const int nCountToBreak = 23;
			for (size_t i = t.wstrName.size() + 2 + nCountToBreak; i < wstr.size(); i += nCountToBreak)
			{
				wstr.insert(i, L"\n");
			}
			if (wstr.back() != L'\n')wstr += L'\n';

			wchar_t sBuffer[16]{};
			swprintf_s(sBuffer, L"%zu/%zu", nTextIndex + 1, m_textData.size());
			wstr += sBuffer;

			if (!t.wstrVoiceFilePath.empty())
			{
				if (m_pAudioPlayer.get() != nullptr)
				{
					m_pAudioPlayer->Play(t.wstrVoiceFilePath.c_str());
				}
			}

			m_textClock.restart();
		}
	}
}
/*文章表示経過時間確認*/
void CKarinchanScenePlayer::checkTextClock()
{
	float fElapsed = m_textClock.getElapsedTime();
	if (::isgreaterequal(fElapsed, 3.f))
	{
		m_textClock.restart();

		if (m_pAudioPlayer.get() != nullptr && m_pAudioPlayer->IsEnded())
		{
			if (!hasReachedLastScene())
			{
				shiftScene(true);
			}
		}
	}
}

void CKarinchanScenePlayer::drawCurrentImage()
{
	if (!m_renderTexture.empty())
	{
		const auto* p = getCurrentImageDatum();
		if (p != nullptr)
		{
			SDxLibRenderTarget renderTargetScope(m_renderTexture.get());
			if (p->isAnimation)
			{
				DxLib::MATRIX matrix = m_dxLibSpinePlayer.calculateTransformMatrix();
				DxLib::SetTransformTo2D(&matrix);

				m_dxLibSpinePlayer.draw();

				DxLib::ResetTransformTo2D();
			}
			else
			{
				size_t nImageIndex = p->stillParams.usIndex;
				if (nImageIndex < m_imageHandles.size())
				{
					const auto& imageHandle = m_imageHandles[nImageIndex];
					const float fScale = p->stillParams.fScale * m_fScale;
					DxLib::MATRIX matrix = calculateTransformMatrixForStill(imageHandle.get(), p->stillParams.fScale * m_fScale);
					DxLib::SetTransformTo2D(&matrix);

					DxLib::DrawGraph(0, 0, imageHandle.get(), FALSE);

					DxLib::ResetTransformTo2D();
				}
			}
		}
		DxLib::SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
		DxLib::DrawGraph(0, 0, m_renderTexture.get(), TRUE);
	}
}

void CKarinchanScenePlayer::drawFormattedText()
{
	if (m_isTextShown)
	{
		m_dxLibTextWriter.draw(m_wstrFormattedText.c_str(), static_cast<unsigned long>(m_wstrFormattedText.size()));
	}
}

DxLib::MATRIX CKarinchanScenePlayer::calculateTransformMatrixForStill(const int imageHandle, const float fScale) const
{
	int iGraphWidth = 0;
	int iGraphsHeight = 0;
	int iRet = DxLib::GetGraphSize(imageHandle, &iGraphWidth, &iGraphsHeight);
	if (iRet == -1)return DxLib::MGetIdent();

	int iTargetWidth = 0;
	int iTargetHeight = 0;
	DxLib::GetDrawScreenSize(&iTargetWidth, &iTargetHeight);
	float fX = (iGraphWidth * fScale - iTargetWidth) / 2 + m_fOffset.u / 2;
	float fY = (iGraphsHeight * fScale - iTargetHeight) / 2 + m_fOffset.v / 2;

	DxLib::MATRIX matrix = DxLib::MGetScale(DxLib::VGet(fScale, fScale, 1.f));
	DxLib::MATRIX tranlateMatrix = DxLib::MGetTranslate(DxLib::VGet(-fX, -fY, 0.f));
	return DxLib::MMult(matrix, tranlateMatrix);
}

void CKarinchanScenePlayer::resetSpinePlayerScale()
{
	m_dxLibSpinePlayer.resetScale();
	m_dxLibSpinePlayer.setSkeletonScale(m_fScale * kDefaultSkeletonScale);
	m_dxLibSpinePlayer.setCanvasScale(m_fScale * kDefaultSkeletonScale);
}

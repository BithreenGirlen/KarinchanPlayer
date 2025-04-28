

#include "karinchan_scene_player.h"

#include "karinchan.h"
#include "win_text.h"

CKarinchanScenePlayer::CKarinchanScenePlayer()
{
	m_dxLibTextWriter.SetFont(L"游明朝", 28, true, true);

	m_pAudioPlayer = std::make_unique<CMfMediaPlayer>();
}

CKarinchanScenePlayer::~CKarinchanScenePlayer()
{

}
/*台本読み込み*/
bool CKarinchanScenePlayer::LoadScenario(const std::wstring& wstrFolderPath)
{
	ClearScenarioData();

	std::vector<adv::ImageFileDatum> imageFileData;

	karinchan::ReadScenario(wstrFolderPath, m_textData, imageFileData, m_sceneData);

	bool bAnimationLoaded = false;
	for (const auto& imageFileDatum : imageFileData)
	{
		if (imageFileDatum.bAnimation)
		{
			if (!bAnimationLoaded)
			{
				std::vector<std::string> atlasPaths;
				std::vector<std::string> skelPaths;

				std::string strAtlasPath = win_text::NarrowUtf8(wstrFolderPath) + "\\animation.atlas.txt";
				std::string strSkelPath = win_text::NarrowUtf8(wstrFolderPath) + "\\animation.json";

				atlasPaths.push_back(strAtlasPath);
				skelPaths.push_back(strSkelPath);

				bAnimationLoaded = m_dxLibSpinePlayer.LoadSpineFromFile(atlasPaths, skelPaths, false);
				if (bAnimationLoaded)
				{
					ResetSpinePlayerScale();
				}
			}

			SImageDatum imageDatum;
			imageDatum.bAnimation = true;
			imageDatum.animationParams.bLoop = imageFileDatum.animationParams.bLoop;
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

			if (dxLibImageHandle.Get() != -1)
			{
				m_imageHandles.push_back(std::move(dxLibImageHandle));

				SImageDatum imageDatum;
				imageDatum.bAnimation = false;
				imageDatum.stillParams.usIndex = static_cast<unsigned short>(m_imageHandles.size() - 1);
				imageDatum.stillParams.fScale = fScale;

				m_imageData.push_back(std::move(imageDatum));
			}
		}
	}

	WorkOutDefaultScale();
	ResetScale();

	PrepareScene();

	m_spineClock.Restart();

	return !m_imageData.empty();
}

void CKarinchanScenePlayer::Update()
{
	float fDelta = m_spineClock.GetElapsedTime();
	const auto* p = GetCurrentImageDatum();
	if (p != nullptr)
	{
		if (p->bAnimation)
		{
			m_dxLibSpinePlayer.Update(fDelta);
		}
	}

	CheckTextClock();

	m_spineClock.Restart();
}

void CKarinchanScenePlayer::Redraw()
{
	DrawCurrentImage();
	DrawFormattedText();
}

void CKarinchanScenePlayer::GetStillImageSize(unsigned int* uiWidth, unsigned int* uiHeight) const
{
	if (uiWidth != nullptr)*uiWidth = static_cast<unsigned int>(kDefaultWidth * m_fScale);
	if (uiHeight != nullptr)*uiHeight = static_cast<unsigned int>(kDefaultHeight * m_fScale);
}
/*場面移行*/
void CKarinchanScenePlayer::ShiftScene(bool bForward)
{
	if (m_sceneData.empty())return;

	if (bForward)
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

	PrepareScene();
}
/*最終場面是否*/
bool CKarinchanScenePlayer::HasReachedLastScene() const
{
	return m_nSceneIndex == m_sceneData.size() - 1;
}
/*文字色切り替え*/
void CKarinchanScenePlayer::ToggleTextColour()
{
	m_dxLibTextWriter.ToggleTextColour();
}
/*尺度変更*/
void CKarinchanScenePlayer::RescaleImage(bool bUpscale)
{
	m_dxLibSpinePlayer.RescaleSkeleton(bUpscale);

	if (bUpscale)
	{
		m_fScale += kfScaleFactor;
	}
	else
	{
		m_fScale -= kfScaleFactor;
		if (m_fScale < kfMinScale)m_fScale = kfMinScale;
	}
}
/*時間尺度変更*/
void CKarinchanScenePlayer::RescaleAnimationTime(bool bFaster)
{
	m_dxLibSpinePlayer.RescaleTime(bFaster);
}
/*視点移動*/
void CKarinchanScenePlayer::MoveViewPoint(int iX, int iY)
{
	const auto* p = GetCurrentImageDatum();
	if (p->bAnimation)
	{
		m_dxLibSpinePlayer.MoveViewPoint(iX, iY);
	}
	else
	{
		m_fOffset.u += iX;
		m_fOffset.v += iY;

		const auto AdjustViewForStill = [this]()
			-> void
			{
				int iClientWidth = 0;
				int iClientHeight = 0;
				DxLib::GetScreenState(&iClientWidth, &iClientHeight, nullptr);

				float fScaledWidth = kDefaultWidth * m_fScale;
				float fScaledHeight = kDefaultHeight * m_fScale;

				float fMaxOffsetX = (fScaledWidth - iClientWidth) / m_fScale;
				float fMaxOffsetY = (fScaledHeight - iClientHeight) / m_fScale;

				m_fOffset.u = (std::max)(-fMaxOffsetX, m_fOffset.u);
				m_fOffset.v = (std::max)(-fMaxOffsetY, m_fOffset.v);

				m_fOffset.u = (std::min)(fMaxOffsetX, m_fOffset.u);
				m_fOffset.v = (std::min)(fMaxOffsetY, m_fOffset.v);
			};

		AdjustViewForStill();
	}
}
/*尺度・位置初期化*/
void CKarinchanScenePlayer::ResetScale()
{
	m_fScale = m_fDefaultScale;
	m_fOffset = {};

	ResetSpinePlayerScale();
}
/*台本データ消去*/
void CKarinchanScenePlayer::ClearScenarioData()
{
	m_textData.clear();

	m_sceneData.clear();
	m_nSceneIndex = 0;

	m_imageData.clear();
	m_imageHandles.clear();

	m_wstrFormattedText.clear();
	m_usLastAnimationIndex = 0;
}
/*標準尺度算出*/
void CKarinchanScenePlayer::WorkOutDefaultScale()
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
CKarinchanScenePlayer::SImageDatum* CKarinchanScenePlayer::GetCurrentImageDatum()
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
void CKarinchanScenePlayer::PrepareScene()
{
	PrepareText();
	CheckAnimationTrack();
}
/*動作切り替わり場面か確認*/
void CKarinchanScenePlayer::CheckAnimationTrack()
{
	const auto* p = GetCurrentImageDatum();
	if (p != nullptr)
	{
		if (p->bAnimation)
		{
			if (m_usLastAnimationIndex != p->animationParams.usIndex)
			{
				m_usLastAnimationIndex = p->animationParams.usIndex;
				m_dxLibSpinePlayer.SetAnimationByIndex(m_usLastAnimationIndex - 1ULL);
			}
		}
		else
		{
			m_usLastAnimationIndex = 0;
		}
	}
}
/*文章作成・附随音声再生*/
void CKarinchanScenePlayer::PrepareText()
{
	if (m_nSceneIndex < m_sceneData.size())
	{
		size_t nTextIndex = m_sceneData[m_nSceneIndex].nTextIndex;
		if (nTextIndex < m_textData.size())
		{
			std::wstring &wstr = m_wstrFormattedText;
			const auto& t = m_textData[nTextIndex];

			wstr = t.wstrName + L"\n" + t.wstrText;
			const int nCountToBreak = 23;
			for (size_t i = t.wstrName.size() + 2 + nCountToBreak; i < wstr.size(); i += nCountToBreak)
			{
				wstr.insert(i, L"\n");
			}
			if (wstr.back() != L'\n')wstr += L"\n";
			wstr += std::to_wstring(nTextIndex + 1) + L"/" + std::to_wstring(m_textData.size());

			if (!t.wstrVoiceFilePath.empty())
			{
				if (m_pAudioPlayer.get() != nullptr)
				{
					m_pAudioPlayer->Play(t.wstrVoiceFilePath.c_str());
				}
			}

			m_textClock.Restart();
		}
	}
}
/*文章表示経過時間確認*/
void CKarinchanScenePlayer::CheckTextClock()
{
	float fElapsed = m_textClock.GetElapsedTime();
	if (::isgreaterequal(fElapsed, 3.f))
	{
		m_textClock.Restart();

		if (m_pAudioPlayer.get() != nullptr && m_pAudioPlayer->IsEnded())
		{
			if (!HasReachedLastScene())
			{
				ShiftScene(true);
			}
		}
	}
}

void CKarinchanScenePlayer::DrawCurrentImage()
{
	const auto* p = GetCurrentImageDatum();
	if (p != nullptr)
	{
		if (p->bAnimation)
		{
			m_dxLibSpinePlayer.Redraw();
		}
		else
		{
			size_t nImageIndex = p->stillParams.usIndex;
			if (nImageIndex < m_imageHandles.size())
			{
				const auto& imageHandle = m_imageHandles[nImageIndex];

				int iGraphWidth = 0;
				int iGraphsHeight = 0;
				int iRet = DxLib::GetGraphSize(imageHandle.Get(), &iGraphWidth, &iGraphsHeight);
				if (iRet == -1)return;

				float fScale = p->stillParams.fScale * m_fScale;
				int iClientWidth = 0;
				int iClientHeight = 0;
				DxLib::GetScreenState(&iClientWidth, &iClientHeight, nullptr);
				float fX = (iGraphWidth * fScale - iClientWidth) / 2 + m_fOffset.u / 2;
				float fY = (iGraphsHeight * fScale - iClientHeight) / 2 + m_fOffset.v / 2;

				DxLib::MATRIX matrix = DxLib::MGetScale(DxLib::VGet(fScale, fScale, 1.f));
				DxLib::MATRIX tranlateMatrix = DxLib::MGetTranslate(DxLib::VGet(-fX, -fY, 0.f));
				matrix = DxLib::MMult(matrix, tranlateMatrix);

				DxLib::SetTransformTo2D(&matrix);

				DxLib::DrawGraph(0, 0, imageHandle.Get(), FALSE);

				DxLib::ResetTransformTo2D();
			}
		}
	}
}

void CKarinchanScenePlayer::DrawFormattedText()
{
	if (m_bTextShown)
	{
		m_dxLibTextWriter.Draw(m_wstrFormattedText.c_str(), static_cast<unsigned long>(m_wstrFormattedText.size()));
	}
}

void CKarinchanScenePlayer::ResetSpinePlayerScale()
{
	/* 静止画の大きさに合わせる。*/

	m_dxLibSpinePlayer.ResetScale();

	float fScaleX = 1.f;
	float fScaleY = 1.f;
	m_dxLibSpinePlayer.FindRootBoneScale(&fScaleX, &fScaleY);
	m_dxLibSpinePlayer.SetZoom(1.f / fScaleX);
}

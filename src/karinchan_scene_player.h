#ifndef KARINCHAN_SCENE_PLAYER_H_
#define KARINCHAN_SCENE_PLAYER_H_

#include <string>
#include <vector>
#include <memory>

#include "adv.h"
#include "dxlib_spine_cpp/dxlib_spine_player.h"
#include "dxlib_clock.h"
#include "dxlib_text_writer.h"
#include "dxlib_handle.h"
#include "mf_media_player.h"

class CKarinchanScenePlayer
{
public:
	CKarinchanScenePlayer();
	~CKarinchanScenePlayer();

	bool LoadScenario(const std::wstring& wstrFolderPath);

	void Update();
	void Redraw();

	void GetStillImageSize(unsigned int* uiWidth, unsigned int* uiHeight) const;

	void ShiftScene(bool bForward);
	bool HasReachedLastScene() const;

	bool GetTextVisibility()const { return m_bTextShown; }
	void SetTextVisibility(bool bShown) { m_bTextShown = bShown; }
	void ToggleTextColour();

	void RescaleImage(bool bUpscale);
	void RescaleAnimationTime(bool bFaster);

	void MoveViewPoint(int iX, int iY);

	void ResetScale();
private:
	using DxLibImageHandle = DxLibHandle<&DxLib::DeleteGraph>;
	static constexpr int kDefaultWidth = 1920;
	static constexpr int kDefaultHeight = 1080;

	static constexpr float kfScaleFactor = 0.025f;
	static constexpr float kfMinScale = 0.15f;

	struct SImageDatum
	{
		bool bAnimation = false;

		struct AnimationParams
		{
			bool bLoop = true;
			unsigned short usIndex = 0;
		};
		AnimationParams animationParams;

		struct StillParams
		{
			unsigned short usIndex = 0;
			float fScale = 1.f;
		};
		StillParams stillParams;
	};

	std::vector<adv::TextDatum> m_textData;
	std::vector<SImageDatum> m_imageData;
	std::vector<DxLibImageHandle> m_imageHandles;

	std::vector<adv::SceneDatum> m_sceneData;
	size_t m_nSceneIndex = 0;

	CDxLibSpinePlayer m_dxLibSpinePlayer;
	CDxLibClock m_spineClock;

	CDxLibTextWriter m_dxLibTextWriter;
	CDxLibClock m_textClock;
	std::unique_ptr<CMfMediaPlayer> m_pAudioPlayer;
	bool m_bTextShown = true;

	std::wstring m_wstrFormattedText;
	unsigned short m_usLastAnimationIndex = 0;

	float m_fDefaultScale = 1.f;

	float m_fScale = 1.f;
	DxLib::FLOAT2 m_fOffset{};

	void ClearScenarioData();
	void WorkOutDefaultScale();

	SImageDatum* GetCurrentImageDatum();

	void PrepareScene();

	void CheckAnimationTrack();
	void PrepareText();

	void CheckTextClock();

	void DrawCurrentImage();
	void DrawFormattedText();

	void ResetSpinePlayerScale();
};

#endif // !KARINCHAN_SCENE_PLAYER_H_

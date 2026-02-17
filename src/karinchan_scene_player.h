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

	bool loadScenario(const std::wstring& folderPath);
	bool hasScenarioData() const;

	void update();
	void draw();

	void getStillImageSize(unsigned int* width, unsigned int* height) const;

	void shiftScene(bool forward);
	bool hasReachedLastScene() const;

	bool isTextShown()const { return m_isTextShown; }
	void setTextVisibility(bool isShown) { m_isTextShown = isShown; }
	void toggleTextColour();

	void rescaleImage(bool upscale);
	void rescaleAnimationTime(short scroll);

	void addOffset(int iX, int iY);

	void resetScale();

	void onResize(int width, int height);

	const std::vector<adv::LabelDatum>& getLabelData() const;
	bool jumpToLabel(size_t nLabelIndex);
private:
	using DxLibImageHandle = DxLibHandle<&DxLib::DeleteGraph>;
	static constexpr int kDefaultWidth = 1920;
	static constexpr int kDefaultHeight = 1080;

	static constexpr float kScaleDelta = 0.025f;
	static constexpr float kMinScale = 0.15f;
	static constexpr float kDefaultSkeletonScale = 1.5f;

	struct SImageDatum
	{
		bool isAnimation = false;

		struct AnimationParams
		{
			bool loop = true;
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

	std::vector<adv::LabelDatum> m_labelData;

	CDxLibSpinePlayer m_dxLibSpinePlayer;
	DxLibImageHandle m_renderTexture = { DxLibImageHandle(-1) };

	CDxLibClock m_spineClock;

	CDxLibTextWriter m_dxLibTextWriter;
	CDxLibClock m_textClock;
	std::unique_ptr<CMfMediaPlayer> m_pAudioPlayer;
	bool m_isTextShown = true;

	std::wstring m_wstrFormattedText;
	unsigned short m_lastAnimationIndex = 0;

	float m_fDefaultScale = 1.f;

	float m_fScale = 1.f;
	DxLib::FLOAT2 m_fOffset{};

	void clearScenarioData();
	void workOutDefaultScale();

	SImageDatum* getCurrentImageDatum();

	void prepareScene();

	void checkAnimationTrack();
	void prepareText();

	void checkTextClock();

	void drawCurrentImage();
	void drawFormattedText();

	DxLib::MATRIX calculateTransformMatrixForStill(const int imageHandle, const float fScale) const;

	void resetSpinePlayerScale();
};

#endif // !KARINCHAN_SCENE_PLAYER_H_

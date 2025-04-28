﻿#ifndef DXLIB_SPINE_PLAYER_H_
#define DXLIB_SPINE_PLAYER_H_

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include "dxlib_spine.h"

class CDxLibSpinePlayer
{
public:
	CDxLibSpinePlayer();
	~CDxLibSpinePlayer();

	bool LoadSpineFromFile(const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelPaths, bool bIsBinary);
	bool LoadSpineFromMemory(const std::vector<std::string>& atlasData, const std::vector<std::string>& atlasPaths, const std::vector<std::string>& skelData, bool bIsBinary);

	bool AddSpineFromFile(const char* szAtlasPath, const char* szSkelPath, bool bBinary);

	void Update(float fDelta);
	void Redraw();

	void RescaleSkeleton(bool bUpscale);
	void RescaleCanvas(bool bUpscale);
	void RescaleTime(bool bHasten);

	void ResetScale();

	void MoveViewPoint(int iX, int iY);

	void ShiftAnimation();
	void ShiftSkin();

	void SetAnimationByIndex(size_t nIndex);
	void SetZoom(float fZoom);
	void FindRootBoneScale(float *fScaleX, float *fScaleY);

	void TogglePma();
	void ToggleBlendModeAdoption();
	bool ToggleDepthBufferValidity();
	void ToggleDrawOrder();

	std::string GetCurrentAnimationNameWithTrackTime(float* fTrackTime = nullptr);

	std::vector<std::string> GetSlotNames();
	const std::vector<std::string>& GetSkinNames() const;
	const std::vector<std::string>& GetAnimationNames() const;

	void SetSlotsToExclude(const std::vector<std::string>& slotNames);
	void MixSkins(const std::vector<std::string>& skinNames);
	void MixAnimations(const std::vector<std::string>& animationNames);

	std::unordered_map<std::string, std::vector<std::string>> GetSlotNamesWithTheirAttachments();
	bool ReplaceAttachment(const char* szSlotName, const char* szAttachmentName);

	void GetBaseSize(float* fWidth, float* fHeight) const;
	float GetCanvasScale() const;
private:
	static constexpr float kfScalePortion = 0.025f;
	static constexpr float kfMinScale = 0.15f;
	enum Constants { kBaseWidth = 1280, kBaseHeight = 720, kMinAtlas = 1024, };

	CDxLibTextureLoader m_textureLoader;
	std::vector<std::unique_ptr<spine::Atlas>> m_atlases;
	std::vector<std::shared_ptr<spine::SkeletonData>> m_skeletonData;
	std::vector<std::shared_ptr<CDxLibSpineDrawable>> m_drawables;

	DxLib::FLOAT2 m_fBaseSize = DxLib::FLOAT2{ kBaseWidth, kBaseHeight };

	float m_fDefaultScale = 1.f;
	DxLib::FLOAT2 m_fDefaultOffset{};

	float m_fTimeScale = 1.f;
	float m_fSkeletonScale = 1.f;
	float m_fCanvasScale = 1.f;
	DxLib::FLOAT2 m_fOffset{};

	std::vector<std::string> m_animationNames;
	size_t m_nAnimationIndex = 0;

	std::vector<std::string> m_skinNames;
	size_t m_nSkinIndex = 0;

	bool m_bDepthBufferEnabled = false;
	bool m_bDrawOrderReversed = false;

	void ClearDrawables();
	bool AddDrawable(spine::SkeletonData* const pSkeletonData);
	bool SetupDrawer();

	void WorkOutDefaultSize();
	void WorkOutDefaultScale();

	void UpdatePosition();
	void UpdateTimeScale();

	void UpdateAnimation();
	void ClearAnimationTracks();

	void SetTransformMatrix() const;
};
#endif // !DXLIB_SPINE_PLAYER_H_

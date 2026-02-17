#ifndef DXLIB_SPINE_PLAYER_H_
#define DXLIB_SPINE_PLAYER_H_

#include "spine_player.h"

class CDxLibSpinePlayer : public CSpinePlayer
{
public:
	CDxLibSpinePlayer();
	~CDxLibSpinePlayer();

	void draw();

	DxLib::MATRIX calculateTransformMatrix() const noexcept;
	DxLib::FLOAT4 getCurrentBoundingOfSlot(const char* slotName, size_t nameLength) const;
	template<size_t nameSize>
	DxLib::FLOAT4 getCurrentBoundingOfSlot(const char(&slotName)[nameSize]) const
	{
		return getCurrentBoundingOfSlot(slotName, nameSize - 1);
	}
private:
	void workOutDefaultScale() override;
	void workOutDefaultOffset() override;
};
#endif // !DXLIB_SPINE_PLAYER_H_

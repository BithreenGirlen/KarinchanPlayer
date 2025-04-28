#ifndef ADV_H_
#define ADV_H_

#include <string>

namespace adv
{
	struct TextDatum
	{
		std::wstring wstrName;
		std::wstring wstrText;
		std::wstring wstrVoiceFilePath;
	};

	struct ImageFileDatum
	{
		bool bAnimation = false;

		struct AnimationParams
		{
			bool bLoop = true;
			unsigned short usIndex = 0;
		};
		AnimationParams animationParams;

		std::wstring wstrFilePath;
	};

	struct SceneDatum
	{
		size_t nTextIndex = 0;
		size_t nImageIndex = 0;
	};
}
#endif // !ADV_H_

#ifndef KARINCHAN_H_
#define KARINCHAN_H_

#include <string>
#include <vector>

#include "adv.h"

namespace karinchan
{
	bool ReadScenario(
		const std::wstring& wstrFolderPath,
		std::vector<adv::TextDatum>& textData,
		std::vector<adv::ImageFileDatum>& imageFileData,
		std::vector<adv::SceneDatum>& sceneData,
		std::vector<adv::LabelDatum>& labelData
	);
}
#endif // !KARINCHAN_H_

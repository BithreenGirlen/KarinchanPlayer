
#include <unordered_map>

#include "karinchan.h"

#include "win_filesystem.h"
#include "win_text.h"
#include "text_utility.h"
#include "path_utility.h"

/* 内部用 */
namespace karinchan
{
	struct SCommandDatum
	{
		std::wstring commandToken;
		std::unordered_map<std::wstring, std::wstring> params;
	};

	/* 指令文解析 */
	static void ParseCommand(const std::wstring& line, SCommandDatum& commandDatum)
	{
		size_t nStart = line.find(L'[');
		size_t nEnd = line.rfind(L']');
		if (nStart == std::wstring::npos || nEnd == std::wstring::npos)return;

		++nStart;
		size_t nPos = line.find(L' ', nStart);
		if (nPos == std::wstring::npos)
		{
			/* 変数指定無し */
			commandDatum.commandToken = line.substr(nStart, nEnd - nStart);
			return;
		}
		else
		{
			commandDatum.commandToken = line.substr(nStart, nPos - nStart);
			++nPos;
		}

		std::wstring paramName;
		std::wstring paramValue;

		for (size_t nRead = nPos; nRead < nEnd; ++nRead)
		{
			const wchar_t p = line[nRead];
			if (p == L' ')
			{
				paramValue = line.substr(nPos, nRead - nPos);
				commandDatum.params.insert({ paramName, paramValue });
				nPos = nRead + 1;

				paramValue.clear();
				paramName.clear();
			}
			else if (p == L'=')
			{
				paramName = line.substr(nPos, nRead - nPos);
				nPos = nRead + 1;
			}
		}

		if (!paramName.empty())
		{
			paramValue = line.substr(nPos, nEnd - nPos);
			commandDatum.params.insert({ paramName, paramValue });
		}
	}

} /* namespace karinchan */

/* 台本読み取り */
bool karinchan::ReadScenario(
	const std::wstring& wstrFolderPath,
	std::vector<adv::TextDatum>& textData,
	std::vector<adv::ImageFileDatum>& imageFileData,
	std::vector<adv::SceneDatum>& sceneData,
	std::vector<adv::LabelDatum>& labelData)
{
	std::wstring wstrScenarioFilePath = wstrFolderPath + L"\\script.txt";

	std::wstring wstrFile = win_text::WidenUtf8(win_filesystem::LoadFileAsString(wstrScenarioFilePath.c_str()));
	if (wstrFile.empty())return false;

	std::vector<std::wstring> lines;
	text_utility::TextToLines(wstrFile, lines);

	adv::TextDatum textDatumBuffer;
	adv::SceneDatum sceneDatumBuffer;
	/* A label to the scene where still or animation will be switched. */
	std::wstring labelBuffer;

	for (const auto& line : lines)
	{
		if (line.size() < 2)continue;
		if (line[0] == L'/' && line[1] == L'/')continue;

		if (line[0] != L'[')
		{
			 /* 地の文・台詞内容 */
			size_t nPos = line.rfind(L"[p]");
			if (nPos != std::wstring::npos)
			{
				/* 区切り */
				textDatumBuffer.wstrText += line.substr(0, nPos);

				textData.push_back(textDatumBuffer);
				textDatumBuffer = adv::TextDatum();

				sceneDatumBuffer.nTextIndex = textData.size() - 1;
				sceneData.push_back(sceneDatumBuffer);

				if (!labelBuffer.empty())
				{
					labelData.emplace_back(adv::LabelDatum{ labelBuffer, sceneData.size() - 1 });
					labelBuffer.clear();
				}

			}
			else
			{
				textDatumBuffer.wstrText += line;
			}
		}
		else
		{
			SCommandDatum commandDatum;
			ParseCommand(line, commandDatum);

			if (commandDatum.commandToken == L"textcmd")
			{
				/* 地の文・台詞開始 */
				if (!commandDatum.params.empty())
				{
					auto iter = commandDatum.params.find(L"name");
					if (iter != commandDatum.params.cend())
					{
						textDatumBuffer.wstrName = iter->second;
						text_utility::ReplaceAll(textDatumBuffer.wstrName, L"#user#", L"一刀");
					}

					iter = commandDatum.params.find(L"voice");
					if (iter != commandDatum.params.cend())
					{
						text_utility::ReplaceAll(iter->second, L"\"", L"");
						textDatumBuffer.wstrVoiceFilePath = wstrFolderPath + L"\\" + path_utility::ExtractFileName(iter->second) + L".m4a";
					}
				}
			}
			else if (commandDatum.commandToken == L"cgstart")
			{
				/* 静止画指定 */
				const auto &iter = commandDatum.params.find(L"address");
				if (iter != commandDatum.params.cend())
				{
					adv::ImageFileDatum imageFileDatum;
					imageFileDatum.isAnimation = false;
					text_utility::ReplaceAll(iter->second, L"\"", L"");
					imageFileDatum.wstrFilePath = wstrFolderPath + L"\\" + path_utility::TruncateFilePath(iter->second);
					
					imageFileData.push_back(std::move(imageFileDatum));
					sceneDatumBuffer.nImageIndex = imageFileData.size() - 1;

					labelBuffer = path_utility::ExtractFileName(iter->second);
				}
			}
			else if (commandDatum.commandToken == L"animestart")
			{
				/* 動作指定 */
				auto iter = commandDatum.params.find(L"address");
				if (iter != commandDatum.params.cend())
				{
					adv::ImageFileDatum imageFileDatum;
					imageFileDatum.isAnimation = true;
					text_utility::ReplaceAll(iter->second, L"\"", L"");
					imageFileDatum.wstrFilePath = wstrFolderPath + L"\\" + path_utility::TruncateFilePath(iter->second);

					const auto &animIter = commandDatum.params.find(L"anim");
					if (animIter == commandDatum.params.cend())continue;
					unsigned long ulIndex = wcstoul(animIter->second.c_str(), nullptr, 10);
					imageFileDatum.animationParams.usIndex = static_cast<unsigned short>(ulIndex);

					iter = commandDatum.params.find(L"loop");
					if (iter == commandDatum.params.cend())continue;
					imageFileDatum.animationParams.loop = iter->second == L"1";

					imageFileData.push_back(std::move(imageFileDatum));
					sceneDatumBuffer.nImageIndex = imageFileData.size() - 1;

					labelBuffer = animIter->first + L"_" + animIter->second;
				}
			}
		}
	}

	return true;
}
